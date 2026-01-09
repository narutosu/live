// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemData.h"
#include "ItemConfig.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common UMETA(DisplayName = "普通"),
	Uncommon UMETA(DisplayName = "优秀"),
	Rare UMETA(DisplayName = "稀有"),
	Epic UMETA(DisplayName = "史诗"),
	Legendary UMETA(DisplayName = "传说")
};

USTRUCT(BlueprintType)
struct FItemDropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	EItemRarity MinRarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	EItemRarity MaxRarity;

	FItemDropEntry()
		: ItemID(0)
		, DropChance(0.0f)
		, MinCount(1)
		, MaxCount(1)
		, MinRarity(EItemRarity::Common)
		, MaxRarity(EItemRarity::Legendary)
	{
	}
};

USTRUCT(BlueprintType)
struct FItemConfigEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 GoldCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemStatModifier> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftClassPtr<class UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftClassPtr<class URPGGameplayAbility> GrantedAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBeSold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBeDropped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBeTraded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText RarityText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FLinearColor RarityColor;

	FItemConfigEntry()
		: ItemID(0)
		, ItemType(EItemType::Consumable)
		, EquipmentSlot(EEquipmentSlot::None)
		, Rarity(EItemRarity::Common)
		, MaxStackSize(1)
		, GoldCost(0)
		, SellPrice(0)
		, RequiredLevel(1)
		, Icon(nullptr)
		, bCanBeSold(true)
		, bCanBeDropped(true)
		, bCanBeTraded(true)
		, RarityText(FText::FromString("普通"))
		, RarityColor(FLinearColor::White)
	{
	}

	//UFUNCTION(BlueprintCallable, Category = "Item")
	FItemData ToItemData() const;
};
