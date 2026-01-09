// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Talent/TalentData.h"
#include "TalentManager.generated.h"

UCLASS()
class LIVE_API UTalentManager : public UObject
{
	GENERATED_BODY()

public:
	static UTalentManager* Get();

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void SetTalentDataTable(UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	FTalentNode GetTalentNode(FName TalentName) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	FTalentNode GetTalentNodeByID(int32 TalentID) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetTalentsByType(ETalentType TalentType) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetAllTalents() const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool HasTalent(FName TalentName) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool HasTalentByID(int32 TalentID) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetAvailableTalents(int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool CanUnlockTalent(int32 TalentID, int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs) const;

private:
	static UTalentManager* Instance;

	UPROPERTY()
	UDataTable* TalentDataTable;
};
