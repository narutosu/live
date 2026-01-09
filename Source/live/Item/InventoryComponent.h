// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemData.h"
#include "Item/ItemInstance.h"
#include "Item/ItemManager.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemName, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemName, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, FName, ItemName, EEquipmentSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnequipped, FName, ItemName, EEquipmentSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY()
	FItemInstance ItemInstance;

	UPROPERTY()
	int32 SlotIndex;

	FInventorySlot()
		: SlotIndex(-1)
	{
	}

	FInventorySlot(int32 InSlotIndex, const FItemInstance& InItemInstance)
		: ItemInstance(InItemInstance)
		, SlotIndex(InSlotIndex)
	{
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIVE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemName, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemName, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(FName ItemName, int32 Count = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipItem(FName ItemName);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UnequipItem(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsEquipped(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FName GetEquippedItemName(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventorySlot> GetInventorySlots() const { return InventorySlots; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetInventorySize() const { return MaxInventorySize; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetUsedSlots() const { return InventorySlots.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasEmptySlot() const { return InventorySlots.Num() < MaxInventorySize; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetGold() const { return Gold; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanAfford(int32 Amount) const { return Gold >= Amount; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemEquipped OnItemEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUnequipped OnItemUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnGoldChanged OnGoldChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxInventorySize;

	TArray<FInventorySlot> InventorySlots;

	TMap<EEquipmentSlot, FName> EquippedItems;

	int32 Gold;

	FInventorySlot* FindInventorySlot(FName ItemName);

	FInventorySlot* FindEmptySlot();

	void ApplyItemEffects(const FItemData& ItemData);

	void RemoveItemEffects(const FItemData& ItemData);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};