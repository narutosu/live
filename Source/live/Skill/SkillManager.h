// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Skill/SkillData.h"
#include "SkillManager.generated.h"

UCLASS()
class LIVE_API USkillManager : public UObject
{
	GENERATED_BODY()

public:
	static USkillManager* Get();

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void SetSkillDataTable(UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	FSkillData GetSkillData(FName SkillName) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	FSkillData GetSkillDataByID(int32 SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FSkillData> GetSkillsByType(ESkillType SkillType) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FSkillData> GetAllSkills() const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool HasSkill(FName SkillName) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool HasSkillByID(int32 SkillID) const;

private:
	static USkillManager* Instance;

	UPROPERTY()
	UDataTable* SkillDataTable;
};
