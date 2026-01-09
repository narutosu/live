// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Talent/TalentData.h"
#include "TalentComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIVE_API UTalentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTalentComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool UnlockTalent(int32 TalentID);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetAvailableTalents();

	UFUNCTION(BlueprintCallable, Category = "Talent")
	TArray<FTalentNode> GetUnlockedTalents();

	UFUNCTION(BlueprintCallable, Category = "Talent")
	int32 GetUnlockedTalentCount() const { return UnlockedTalentIDs.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool IsTalentUnlocked(int32 TalentID) const { return UnlockedTalentIDs.Contains(TalentID); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TSoftObjectPtr<UTalentData> TalentDataAsset;

	TSet<int32> UnlockedTalentIDs;

	TMap<int32, int32> TalentRanks;

	UFUNCTION()
	void OnOwnerLevelChanged(int32 NewLevel, int32 OldLevel);

	void ApplyTalentEffect(const FTalentNode& TalentNode);

	void RemoveTalentEffect(const FTalentNode& TalentNode);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};