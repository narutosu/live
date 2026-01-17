// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemManager.h"
#include "Item/ItemActor.h"

UItemManager* UItemManager::Instance = nullptr;

UItemManager* UItemManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UItemManager>();
		Instance->AddToRoot();
		Instance->Initialize();
	}
	return Instance;
}

void UItemManager::Initialize()
{
	if (!ItemDataTable)
	{
		ItemDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Script/Engine.DataTable'/Game/DT/DT_Item.DT_Item'"));
	}
}

void UItemManager::SetItemDataTable(UDataTable* InDataTable)
{
	ItemDataTable = InDataTable;
}

FItemData UItemManager::GetItemData(FName ItemName) const
{
	if (ItemDataTable)
	{
		const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemName, TEXT("ItemName"));
		if (ItemData)
		{
			return *ItemData;
		}
	}
	return FItemData();
}

FItemData UItemManager::GetItemDataByID(int32 ItemID) const
{
	if (ItemDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(ItemDataTable->GetRowMap()); It; ++It)
		{
			const FItemData* ItemData = reinterpret_cast<const FItemData*>(It.Value());
			if (ItemData && ItemData->ItemID == ItemID)
			{
				return *ItemData;
			}
		}
	}
	return FItemData();
}

TArray<FItemData> UItemManager::GetItemsByType(EItemType ItemType) const
{
	TArray<FItemData> Result;

	if (ItemDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(ItemDataTable->GetRowMap()); It; ++It)
		{
			const FItemData* ItemData = reinterpret_cast<const FItemData*>(It.Value());
			if (ItemData && ItemData->ItemType == ItemType)
			{
				Result.Add(*ItemData);
			}
		}
	}

	return Result;
}

TArray<FItemData> UItemManager::GetAllItems() const
{
	TArray<FItemData> Result;

	if (ItemDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(ItemDataTable->GetRowMap()); It; ++It)
		{
			const FItemData* ItemData = reinterpret_cast<const FItemData*>(It.Value());
			if (ItemData)
			{
				Result.Add(*ItemData);
			}
		}
	}

	return Result;
}

bool UItemManager::HasItem(FName ItemName) const
{
	FItemData ItemData = GetItemData(ItemName);
	return ItemData.ItemID > 0;
}

bool UItemManager::HasItemByID(int32 ItemID) const
{
	FItemData ItemData = GetItemDataByID(ItemID);
	return ItemData.ItemID > 0;
}

FItemInstance UItemManager::CreateItemInstance(FName ItemName, int32 Count) const
{
	FItemData ItemData = GetItemData(ItemName);
	if (ItemData.ItemID > 0)
	{
		return FItemInstance(ItemData.ItemID, ItemName, Count);
	}
	return FItemInstance();
}

FItemInstance UItemManager::CreateItemInstanceByID(int32 ItemID, int32 Count) const
{
	FItemData ItemData = GetItemDataByID(ItemID);
	if (ItemData.ItemID > 0)
	{
		return FItemInstance(ItemData.ItemID, ItemData.ItemName, Count);
	}
	return FItemInstance();
}

bool UItemManager::IsValidItem(FName ItemName) const
{
	return HasItem(ItemName);
}

bool UItemManager::IsValidItemID(int32 ItemID) const
{
	return HasItemByID(ItemID);
}

AItemActor* UItemManager::DropItem(UObject* WorldContextObject, const FVector& Location, FName ItemName)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// Get the item data for the specified item
		FItemData ItemData = Get()->GetItemData(ItemName);
		
		// Check if the item is valid
		if (ItemData.ItemID == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item not found: %s"), *ItemName.ToString());
			return nullptr;
		}

		// Spawn the ItemActor
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AItemActor* DroppedItem = World->SpawnActor<AItemActor>(AItemActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
		
		if (DroppedItem)
		{
			// Set the item name and setup appearance
			DroppedItem->SetItemName(ItemName);
			DroppedItem->SetupItemAppearance();
			
			UE_LOG(LogTemp, Log, TEXT("Dropped item: %s at location: %s"), *ItemName.ToString(), *Location.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn ItemActor at location: %s"), *Location.ToString());
		}
		FVector Offset = FVector(0,0,-50);//这个可以加到配置,目的是让物品掉到贴近地面
		DroppedItem->PlayDropAnimation(DroppedItem->GetActorLocation()+Offset, 300.0f, 200.0f, 1.0f);
		return DroppedItem;
	}

	return nullptr;
}

AItemActor* UItemManager::DropRandomItem(UObject* WorldContextObject, const FVector& Location, EItemType ItemType)
{
	// Get all available items
	TArray<FItemData> AvailableItems;
	
	// If ItemType is Consumable, drop from all items, otherwise filter by type
	if (ItemType == EItemType::Consumable)
	{
		AvailableItems = GetAllItems();
	}
	else
	{
		AvailableItems = GetItemsByType(ItemType);
	}

	// Check if there are any items to drop
	if (AvailableItems.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No items available to drop"));
		return nullptr;
	}

	// Select a random item
	int32 RandomIndex = FMath::RandRange(0, AvailableItems.Num() - 1);
	FName RandomItemName = AvailableItems[RandomIndex].ItemName;

	// Reuse DropItem to spawn the item
	return DropItem(WorldContextObject, Location, RandomItemName);
}