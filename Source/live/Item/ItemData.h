// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Consumable UMETA(DisplayName = "消耗品"),
	Equipment UMETA(DisplayName = "装备"),
	Material UMETA(DisplayName = "材料"),
	Quest UMETA(DisplayName = "任务物品")
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None UMETA(DisplayName = "无"),
	Weapon UMETA(DisplayName = "武器"),
	Armor UMETA(DisplayName = "护甲"),
	Helmet UMETA(DisplayName = "头盔"),
	Boots UMETA(DisplayName = "鞋子"),
	Accessory UMETA(DisplayName = "饰品")
};

USTRUCT(BlueprintType)
struct FItemStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGameplayTag AttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Value;

	FItemStatModifier()
		: Value(0.0f)
	{
	}
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 GoldCost;

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

	FItemData()
		: ItemID(0)
		, ItemName(NAME_None)
		, ItemType(EItemType::Consumable)
		, EquipmentSlot(EEquipmentSlot::None)
		, MaxStackSize(1)
		, GoldCost(0)
		, Icon(nullptr)
		, bCanBeSold(true)
		, bCanBeDropped(true)
	{
	}
};