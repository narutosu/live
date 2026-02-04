// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "SkillData.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Active UMETA(DisplayName = "主动技能"),
	Passive UMETA(DisplayName = "被动技能"),
	Ultimate UMETA(DisplayName = "大招")
};

UENUM(BlueprintType)
enum class ESkillTargetType : uint8
{
	None UMETA(DisplayName = "无目标"),
	Unit UMETA(DisplayName = "单位目标"),
	Point UMETA(DisplayName = "地点目标"),
	Direction UMETA(DisplayName = "方向目标"),
	Area UMETA(DisplayName = "区域目标")
};

USTRUCT(BlueprintType)
struct FSkillLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float ManaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Radius;

	FSkillLevelData()
		: Level(1)
		, ManaCost(0.0f)
		, Cooldown(0.0f)
		, Damage(0.0f)
		, Duration(0.0f)
		, Radius(0.0f)
	{
	}
};

USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 SkillID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillType SkillType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillTargetType TargetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 MaxLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<int32> RequiredLevels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FSkillLevelData> LevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSoftClassPtr<class URPGGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FGameplayTagContainer SkillTags;

	FSkillData()
		: SkillID(0)
		, SkillName(NAME_None)
		, SkillType(ESkillType::Active)
		, TargetType(ESkillTargetType::None)
		, MaxLevel(4)
		, Icon(nullptr)
		, RequiredLevels(TArray<int32>({ 1,3, 5, 7 }))
	{
	}
	FSkillLevelData GetLevelData(int32 Level) const;
};