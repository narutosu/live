// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/InventoryComponent.h"
#include "Item/ItemManager.h"
#include "Role/RoleBase.h"
#include "AbilitySystemComponent.h"
#include "GameHelper.h"
#include "GAS/Common/RPGGameplayAbility.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	MaxInventorySize = 20;
	Gold = 0;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UInventoryComponent::AddItem(FName ItemName, int32 Count)
{
	if (Count <= 0)
	{
		return false;
	}
	
	FItemData ItemData = UItemManager::Get()->GetItemData(ItemName);
	if (ItemData.ItemID == 0)
	{
		return false;
	}

	int32 RemainingCount = Count;
	
	// 显示 Toast 消息
	FString Message = FString::Printf(TEXT("Add %d %s"), Count, *ItemName.ToString());
	UGameHelper::ShowToast(GetOwner(), Message);
	
	if (ItemData.MaxStackSize > 1)
	{
		FInventorySlot* ExistingSlot = FindInventorySlot(ItemName);
		if (ExistingSlot)
		{
			int32 CanAdd = ItemData.MaxStackSize - ExistingSlot->ItemInstance.StackCount;
			int32 ToAdd = FMath::Min(CanAdd, RemainingCount);
			ExistingSlot->ItemInstance.AddStack(ToAdd);
			RemainingCount -= ToAdd;
			OnItemAdded.Broadcast(ItemName, ExistingSlot->ItemInstance.StackCount);
		}
	}

	while (RemainingCount > 0 && HasEmptySlot())
	{
		FInventorySlot* EmptySlot = FindEmptySlot();
		if (!EmptySlot)
		{
			break;
		}

		int32 ToAdd = ItemData.MaxStackSize > 1 ? FMath::Min(ItemData.MaxStackSize, RemainingCount) : 1;
		EmptySlot->ItemInstance = FItemInstance(ItemData.ItemID, ItemName, ToAdd);
		RemainingCount -= ToAdd;
		OnItemAdded.Broadcast(ItemName, EmptySlot->ItemInstance.StackCount);
	}

	return RemainingCount == 0;
}

bool UInventoryComponent::RemoveItem(FName ItemName, int32 Count)
{
	if (Count <= 0)
	{
		return false;
	}

	int32 TotalCount = GetItemCount(ItemName);
	if (TotalCount < Count)
	{
		return false;
	}

	int32 RemainingToRemove = Count;
	for (int32 i = InventorySlots.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
	{
		FInventorySlot& Slot = InventorySlots[i];
		if (Slot.ItemInstance.ItemName == ItemName)
		{
			int32 CanRemove = FMath::Min(Slot.ItemInstance.StackCount, RemainingToRemove);
			Slot.ItemInstance.RemoveStack(CanRemove);
			RemainingToRemove -= CanRemove;

			if (Slot.ItemInstance.StackCount <= 0)
			{
				InventorySlots.RemoveAt(i);
			}

			OnItemRemoved.Broadcast(ItemName, Slot.ItemInstance.StackCount);
		}
	}

	return true;
}

bool UInventoryComponent::HasItem(FName ItemName, int32 Count) const
{
	return GetItemCount(ItemName) >= Count;
}

int32 UInventoryComponent::GetItemCount(FName ItemName) const
{
	int32 TotalCount = 0;
	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemInstance.ItemName == ItemName)
		{
			TotalCount += Slot.ItemInstance.StackCount;
		}
	}
	return TotalCount;
}

bool UInventoryComponent::EquipItem(FName ItemName)
{
	FItemData ItemData = UItemManager::Get()->GetItemData(ItemName);
	if (ItemData.ItemID == 0 || ItemData.ItemType != EItemType::Equipment || ItemData.EquipmentSlot == EEquipmentSlot::None)
	{
		return false;
	}

	FInventorySlot* Slot = FindInventorySlot(ItemName);
	if (!Slot)
	{
		return false;
	}

	EEquipmentSlot TargetSlot = ItemData.EquipmentSlot;

	if (EquippedItems.Contains(TargetSlot))
	{
		FName OldEquippedItemName = EquippedItems[TargetSlot];
		UnequipItem(TargetSlot);
	}

	Slot->ItemInstance.bIsEquipped = true;
	EquippedItems.Add(TargetSlot, ItemName);

	ApplyItemEffects(ItemData);

	OnItemEquipped.Broadcast(ItemName, TargetSlot);

	return true;
}

bool UInventoryComponent::UnequipItem(EEquipmentSlot Slot)
{
	if (!EquippedItems.Contains(Slot))
	{
		return false;
	}

	FName ItemName = EquippedItems[Slot];
	FInventorySlot* InventorySlot = FindInventorySlot(ItemName);
	if (InventorySlot)
	{
		InventorySlot->ItemInstance.bIsEquipped = false;
	}

	FItemData ItemData = UItemManager::Get()->GetItemData(ItemName);
	if (ItemData.ItemID > 0)
	{
		RemoveItemEffects(ItemData);
	}

	EquippedItems.Remove(Slot);

	OnItemUnequipped.Broadcast(ItemName, Slot);

	return true;
}

bool UInventoryComponent::IsEquipped(FName ItemName) const
{
	for (const TPair<EEquipmentSlot, FName>& Pair : EquippedItems)
	{
		if (Pair.Value == ItemName)
		{
			return true;
		}
	}
	return false;
}

FName UInventoryComponent::GetEquippedItemName(EEquipmentSlot Slot) const
{
	if (EquippedItems.Contains(Slot))
	{
		return EquippedItems[Slot];
	}
	return NAME_None;
}



bool UInventoryComponent::AddGold(int32 Amount)
{
	if (Amount < 0)
	{
		return false;
	}

	Gold += Amount;
	OnGoldChanged.Broadcast(Gold);
	return true;
}

bool UInventoryComponent::RemoveGold(int32 Amount)
{
	if (Amount < 0 || Gold < Amount)
	{
		return false;
	}

	Gold -= Amount;
	OnGoldChanged.Broadcast(Gold);
	return true;
}



FInventorySlot* UInventoryComponent::FindInventorySlot(FName ItemName)
{
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemInstance.ItemName == ItemName)
		{
			return &Slot;
		}
	}
	return nullptr;
}

FInventorySlot* UInventoryComponent::FindEmptySlot()
{
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemInstance.ItemName.IsNone())
		{
			return &Slot;
		}
	}

	if (InventorySlots.Num() < MaxInventorySize)
	{
		int32 NewIndex = InventorySlots.Num();
		InventorySlots.Add(FInventorySlot(NewIndex, FItemInstance()));
		return &InventorySlots.Last();
	}

	return nullptr;
}

void UInventoryComponent::ApplyItemEffects(const FItemData& ItemData)
{
	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return;
	}

	UAbilitySystemComponent* ASC = RoleOwner->GetAbilitySystemComponent();

	if (ItemData.GameplayEffectClass.IsValid())
	{
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			ItemData.GameplayEffectClass.LoadSynchronous(),
			RoleOwner->GetCharacterLevel(),
			ASC->MakeEffectContext()
		);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		}
	}

	if (ItemData.GrantedAbility.IsValid())
	{
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(
			FGameplayAbilitySpec(ItemData.GrantedAbility.LoadSynchronous(), 1, -1, RoleOwner)
		);

		FInventorySlot* Slot = FindInventorySlot(ItemData.ItemName);
		if (Slot)
		{
			Slot->ItemInstance.GrantedAbilityHandle = Handle;
		}
	}
}

void UInventoryComponent::RemoveItemEffects(const FItemData& ItemData)
{
	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return;
	}

	FInventorySlot* Slot = FindInventorySlot(ItemData.ItemName);
	if (Slot && Slot->ItemInstance.GrantedAbilityHandle.IsValid())
	{
		RoleOwner->GetAbilitySystemComponent()->ClearAbility(Slot->ItemInstance.GrantedAbilityHandle);
		Slot->ItemInstance.GrantedAbilityHandle = FGameplayAbilitySpecHandle();
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventorySlots);
	DOREPLIFETIME(UInventoryComponent, EquippedItems);
	DOREPLIFETIME(UInventoryComponent, Gold);
}