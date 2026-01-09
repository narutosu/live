// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemDropSystem.h"
#include "Item/ItemConfigAsset.h"
#include "Role/RoleBase.h"
#include "Item/InventoryComponent.h"

UItemDropSystem::UItemDropSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	DefaultDropRateMultiplier = 1.0f;
	DroppedItemLifeTime = 300.0f;
}

void UItemDropSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UItemDropSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDroppedItems(DeltaTime);
}

TArray<int32> UItemDropSystem::GenerateDrops(const TArray<FItemDropEntry>& DropTable, int32 KillerLevel, float DropRateMultiplier)
{
	TArray<int32> DroppedItemIDs;

	if (!ItemConfigAsset.IsValid())
	{
		return DroppedItemIDs;
	}

	UItemConfigAsset* Config = ItemConfigAsset.LoadSynchronous();
	if (!Config)
	{
		return DroppedItemIDs;
	}

	float ActualMultiplier = DefaultDropRateMultiplier * DropRateMultiplier;

	for (const FItemDropEntry& DropEntry : DropTable)
	{
		const FItemConfigEntry* ItemConfig = Config->GetItemConfig(DropEntry.ItemID);
		if (!ItemConfig)
		{
			continue;
		}

		if (ItemConfig->RequiredLevel > KillerLevel)
		{
			continue;
		}

		float AdjustedDropChance = DropEntry.DropChance * ActualMultiplier;
		float RandomChance = FMath::FRand() * 100.0f;

		if (RandomChance <= AdjustedDropChance)
		{
			int32 Count = FMath::RandRange(DropEntry.MinCount, DropEntry.MaxCount);
			for (int32 i = 0; i < Count; ++i)
			{
				DroppedItemIDs.Add(DropEntry.ItemID);
			}
		}
	}

	return DroppedItemIDs;
}

TArray<int32> UItemDropSystem::GenerateDropsFromConfig(int32 KillerLevel, float DropRateMultiplier)
{
	if (!ItemConfigAsset.IsValid())
	{
		return TArray<int32>();
	}

	UItemConfigAsset* Config = ItemConfigAsset.LoadSynchronous();
	if (!Config)
	{
		return TArray<int32>();
	}

	return GenerateDrops(Config->DefaultDropTable, KillerLevel, DropRateMultiplier);
}

void UItemDropSystem::SpawnDroppedItem(int32 ItemID, int32 Count, const FVector& Location, const FRotator& Rotation)
{
	FDroppedItem NewDroppedItem;
	NewDroppedItem.ItemID = ItemID;
	NewDroppedItem.Count = Count;
	NewDroppedItem.Location = Location;
	NewDroppedItem.Rotation = Rotation;
	NewDroppedItem.LifeTime = DroppedItemLifeTime;
	NewDroppedItem.bHasBeenPickedUp = false;

	DroppedItems.Add(NewDroppedItem);

	OnItemDropped.Broadcast(ItemID, Count);
}

bool UItemDropSystem::PickupItem(AActor* Picker, int32 ItemID, int32 Count)
{
	ARoleBase* RolePicker = Cast<ARoleBase>(Picker);
	if (!RolePicker)
	{
		return false;
	}

	UInventoryComponent* Inventory = RolePicker->GetInventoryComponent();
	if (!Inventory)
	{
		return false;
	}

	for (int32 i = DroppedItems.Num() - 1; i >= 0; --i)
	{
		FDroppedItem& DroppedItem = DroppedItems[i];
		if (DroppedItem.ItemID == ItemID && !DroppedItem.bHasBeenPickedUp)
		{
			int32 CountToPickup = FMath::Min(Count, DroppedItem.Count);

			if (Inventory->AddItem(ItemID, CountToPickup))
			{
				DroppedItem.Count -= CountToPickup;
				DroppedItem.bHasBeenPickedUp = true;

				OnItemPickedUp.Broadcast(ItemID, CountToPickup);

				if (DroppedItem.Count <= 0)
				{
					DroppedItems.RemoveAt(i);
				}

				return true;
			}
		}
	}

	return false;
}

void UItemDropSystem::UpdateDroppedItems(float DeltaTime)
{
	for (int32 i = DroppedItems.Num() - 1; i >= 0; --i)
	{
		FDroppedItem& Item = DroppedItems[i];
		Item.LifeTime -= DeltaTime;

		if (Item.LifeTime <= 0.0f)
		{
			DroppedItems.RemoveAt(i);
		}
	}
}

void UItemDropSystem::ClearDroppedItems()
{
	DroppedItems.Empty();
}

void UItemDropSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemDropSystem, DroppedItems);
}
