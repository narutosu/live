// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemData.h"
#include "GameplayAbilitySpecHandle.h"
#include "ItemInstance.generated.h"

USTRUCT(BlueprintType)
struct FItemInstance
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID;

	UPROPERTY()
	FName ItemName;

	UPROPERTY()
	int32 StackCount;

	UPROPERTY()
	int32 Durability;

	UPROPERTY()
	bool bIsEquipped;

	UPROPERTY()
	FGameplayAbilitySpecHandle GrantedAbilityHandle;

	FItemInstance()
		: ItemID(0)
		, ItemName(NAME_None)
		, StackCount(1)
		, Durability(100)
		, bIsEquipped(false)
	{
	}

	FItemInstance(int32 InItemID, FName InItemName, int32 InStackCount = 1)
		: ItemID(InItemID)
		, ItemName(InItemName)
		, StackCount(InStackCount)
		, Durability(100)
		, bIsEquipped(false)
	{
	}

	bool IsStackableWith(const FItemInstance& Other) const
	{
		return ItemID == Other.ItemID && ItemName == Other.ItemName && !bIsEquipped && !Other.bIsEquipped;
	}

	void AddStack(int32 Amount)
	{
		StackCount += Amount;
	}

	void RemoveStack(int32 Amount)
	{
		StackCount = FMath::Max(0, StackCount - Amount);
	}
};