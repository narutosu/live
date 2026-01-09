// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemManager.h"

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
		const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemName, TEXT("GetItemData"));
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