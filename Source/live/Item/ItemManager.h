// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Item/ItemData.h"
#include "Item/ItemInstance.h"
#include "ItemManager.generated.h"

UCLASS()
class LIVE_API UItemManager : public UObject
{
	GENERATED_BODY()

public:
	static UItemManager* Get();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDataTable(UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemData GetItemData(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemData GetItemDataByID(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<FItemData> GetItemsByType(EItemType ItemType) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<FItemData> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool HasItem(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool HasItemByID(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemInstance CreateItemInstance(FName ItemName, int32 Count = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemInstance CreateItemInstanceByID(int32 ItemID, int32 Count = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool IsValidItem(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool IsValidItemID(int32 ItemID) const;

private:
	static UItemManager* Instance;

	UPROPERTY()
	UDataTable* ItemDataTable;
};