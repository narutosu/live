// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemConfig.h"
#include "ItemConfigAsset.generated.h"

UCLASS()
class LIVE_API UItemConfigAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemConfigEntry> AllItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FItemDropEntry> DefaultDropTable;

	UFUNCTION(BlueprintCallable, Category = "Item")
	const FItemConfigEntry GetItemConfig(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<FItemConfigEntry> GetItemsByType(EItemType ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<FItemConfigEntry> GetItemsByRarity(EItemRarity Rarity) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<FItemConfigEntry> GetShopItems(int32 MinLevel = 0, int32 MaxLevel = 999) const;

	UFUNCTION(BlueprintCallable, Category = "Drop")
	TArray<int32> GenerateRandomDrops(const TArray<FItemDropEntry>& DropTable, int32 MinLevel = 0, int32 MaxLevel = 999) const;

	UFUNCTION(BlueprintCallable, Category = "Drop")
	TArray<int32> GenerateRandomDropsFromDefault(int32 MinLevel = 0, int32 MaxLevel = 999) const;
};
