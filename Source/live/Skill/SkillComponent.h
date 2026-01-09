// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillData.h"
#include "Skill/SkillManager.h"
#include "SkillComponent.generated.h"

USTRUCT(BlueprintType)
struct FActiveSkill
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SkillID;

	UPROPERTY()
	int32 CurrentLevel;

	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY()
	float CurrentCooldown;

	UPROPERTY()
	float LastCastTime;

	FActiveSkill()
		: SkillID(0)
		, CurrentLevel(0)
		, SlotIndex(-1)
		, CurrentCooldown(0.0f)
		, LastCastTime(0.0f)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillLevelChanged, int32, SkillID, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCooldownChanged, int32, SkillID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIVE_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool LearnSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool LearnSkillByName(FName SkillName);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool UpgradeSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CastSkill(int32 SkillID, AActor* Target = nullptr, const FVector& TargetLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CanCastSkill(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillCooldown(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillCooldownRemaining(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	int32 GetSkillLevel(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FActiveSkill> GetLearnedSkills() const { return ActiveSkills; }

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool HasSkill(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CastSkillByName(FName SkillName, AActor* Target = nullptr, const FVector& TargetLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CastSkillBySlot(int32 SlotIndex, AActor* Target = nullptr, const FVector& TargetLocation = FVector::ZeroVector);

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillLevelChanged OnSkillLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillCooldownChanged OnSkillCooldownChanged;

protected:
	UPROPERTY(Replicated)
	TArray<FActiveSkill> ActiveSkills;

	UFUNCTION()
	void OnOwnerLevelChanged(int32 NewLevel, int32 OldLevel);

	FActiveSkill* FindActiveSkill(int32 SkillID);
	const FActiveSkill* FindActiveSkill(int32 SkillID) const;

	void GrantAbility(int32 SkillID, const FSkillData& SkillData);

	void RemoveAbility(int32 SkillID);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};