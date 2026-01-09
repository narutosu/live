// Fill out your copyright notice in the Description page of Project Settings.

#include "Talent/TalentData.h"
#include "Talent/TalentManager.h"

FTalentNode UTalentData::GetTalentNode(int32 TalentID)
{
	return UTalentManager::Get()->GetTalentNodeByID(TalentID);
}

TArray<FTalentNode> UTalentData::GetAvailableTalents(int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs)
{
	return UTalentManager::Get()->GetAvailableTalents(CurrentLevel, UnlockedTalentIDs);
}

bool UTalentData::CanUnlockTalent(int32 TalentID, int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs)
{
	return UTalentManager::Get()->CanUnlockTalent(TalentID, CurrentLevel, UnlockedTalentIDs);
}