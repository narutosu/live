// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "TalentData.generated.h"

UENUM(BlueprintType)
enum class ETalentType : uint8
{
	Attribute UMETA(DisplayName = "属性天赋"),
	SkillEnhancement UMETA(DisplayName = "技能增强"),
	Special UMETA(DisplayName = "特殊天赋")
};

USTRUCT(BlueprintType)
struct FTalentNode : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	int32 TalentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FName TalentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	ETalentType TalentType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	int32 MaxRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TArray<int32> PrerequisiteTalentIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TMap<FGameplayTag, float> AttributeModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TSoftClassPtr<class UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	bool bIsLeftBranch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	int32 BranchPosition;

	FTalentNode()
		: TalentID(0)
		, TalentName(NAME_None)
		, TalentType(ETalentType::Attribute)
		, RequiredLevel(10)
		, MaxRank(1)
		, bIsLeftBranch(false)
		, BranchPosition(0)
	{
	}
};

UCLASS(BlueprintType, Blueprintable)
class LIVE_API UTalentData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TArray<FTalentNode> TalentTree;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	FTalentNode GetTalentNode(int32 TalentID);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetAvailableTalents(int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool CanUnlockTalent(int32 TalentID, int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs);
};
