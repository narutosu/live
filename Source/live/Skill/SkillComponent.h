// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillData.h"
#include "Skill/SkillManager.h"

class FDelegateHandle;

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

	// Store delegate handles for cooldown tag events
	TArray<FDelegateHandle> CooldownDelegateHandles;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSkillCooldownChangedByName, FName, SkillName, float, TimeRemaining, float, CooldownDuration, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSkillCooldownChangedBySlot, int32, SlotIndex, float, TimeRemaining, float, CooldownDuration, FName, SkillName);

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
	bool UpgradeSkillBySlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ForgetSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ForgetSkillByName(FName SkillName);

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

	// Get cooldown time remaining and duration by SkillName
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void GetSkillCooldownTimeRemainingAndDurationByName(FName SkillName, float& OutTimeRemaining, float& OutCooldownDuration) const;

	// Get cooldown time remaining and duration by SlotIndex
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void GetSkillCooldownTimeRemainingAndDurationBySlot(int32 SlotIndex, float& OutTimeRemaining, float& OutCooldownDuration) const;

	// Register callback for skill cooldown changes by SkillName
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RegisterSkillCooldownCallbackByName(FName SkillName);

	// Register callback for skill cooldown changes by SlotIndex
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RegisterSkillCooldownCallbackBySlot(int32 SlotIndex);

	// Unregister callback for skill cooldown changes by SkillName
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void UnregisterSkillCooldownCallbackByName(FName SkillName);

	// Unregister callback for skill cooldown changes by SlotIndex
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void UnregisterSkillCooldownCallbackBySlot(int32 SlotIndex);

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillLevelChanged OnSkillLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillCooldownChanged OnSkillCooldownChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillCooldownChangedByName OnSkillCooldownChangedByName;

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillCooldownChangedBySlot OnSkillCooldownChangedBySlot;

protected:
	UPROPERTY(Replicated)
	TArray<FActiveSkill> ActiveSkills;

	UFUNCTION()
	void OnOwnerLevelChanged(int32 NewLevel, int32 OldLevel);

	FActiveSkill* FindActiveSkill(int32 SkillID);
	const FActiveSkill* FindActiveSkill(int32 SkillID) const;

	FActiveSkill* FindActiveSkillByName(FName SkillName);
	const FActiveSkill* FindActiveSkillByName(FName SkillName) const;

	FActiveSkill* FindActiveSkillBySlot(int32 SlotIndex);
	const FActiveSkill* FindActiveSkillBySlot(int32 SlotIndex) const;

	void GrantAbility(int32 SkillID, const FSkillData& SkillData);

	void RemoveAbility(int32 SkillID);

	// Callback for gameplay tag events (cooldown changes)
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);

	// Register cooldown tag event for a specific skill
	void RegisterCooldownTagEventForSkill(FActiveSkill& ActiveSkill);

	// Unregister cooldown tag event for a specific skill
	void UnregisterCooldownTagEventForSkill(FActiveSkill& ActiveSkill);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};