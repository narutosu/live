// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemConfigAsset.h"
#include "Kismet/KismetMathLibrary.h"

const FItemConfigEntry* UItemConfigAsset::GetItemConfig(int32 ItemID) const
{
	for (const FItemConfigEntry& Item : AllItems)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

TArray<FItemConfigEntry> UItemConfigAsset::GetItemsByType(EItemType ItemType) const
{
	TArray<FItemConfigEntry> Result;
	for (const FItemConfigEntry& Item : AllItems)
	{
		if (Item.ItemType == ItemType)
		{
			Result.Add(Item);
		}
	}
	return Result;
}

TArray<FItemConfigEntry> UItemConfigAsset::GetItemsByRarity(EItemRarity Rarity) const
{
	TArray<FItemConfigEntry> Result;
	for (const FItemConfigEntry& Item : AllItems)
	{
		if (Item.Rarity == Rarity)
		{
			Result.Add(Item);
		}
	}
	return Result;
}

TArray<FItemConfigEntry> UItemConfigAsset::GetShopItems(int32 MinLevel, int32 MaxLevel) const
{
	TArray<FItemConfigEntry> Result;
	for (const FItemConfigEntry& Item : AllItems)
	{
		if (Item.RequiredLevel >= MinLevel && Item.RequiredLevel <= MaxLevel && Item.bCanBeSold)
		{
			Result.Add(Item);
		}
	}
	return Result;
}

TArray<int32> UItemConfigAsset::GenerateRandomDrops(const TArray<FItemDropEntry>& DropTable, int32 MinLevel, int32 MaxLevel) const
{
	TArray<int32> DroppedItems;

	for (const FItemDropEntry& DropEntry : DropTable)
	{
		const FItemConfigEntry* ItemConfig = GetItemConfig(DropEntry.ItemID);
		if (!ItemConfig)
		{
			continue;
		}

		if (ItemConfig->RequiredLevel < MinLevel || ItemConfig->RequiredLevel > MaxLevel)
		{
			continue;
		}

		float RandomChance = FMath::FRand() * 100.0f;
		if (RandomChance <= DropEntry.DropChance)
		{
			int32 Count = FMath::RandRange(DropEntry.MinCount, DropEntry.MaxCount);
			for (int32 i = 0; i < Count; ++i)
			{
				DroppedItems.Add(DropEntry.ItemID);
			}
		}
	}

	return DroppedItems;
}

TArray<int32> UItemConfigAsset::GenerateRandomDropsFromDefault(int32 MinLevel, int32 MaxLevel) const
{
	return GenerateRandomDrops(DefaultDropTable, MinLevel, MaxLevel);
}
