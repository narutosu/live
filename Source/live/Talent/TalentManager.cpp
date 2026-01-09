// Fill out your copyright notice in the Description page of Project Settings.

#include "Talent/TalentManager.h"

UTalentManager* UTalentManager::Instance = nullptr;

UTalentManager* UTalentManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UTalentManager>();
		Instance->AddToRoot();
		Instance->Initialize();
	}
	return Instance;
}

void UTalentManager::Initialize()
{
	if (!TalentDataTable)
	{
		TalentDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DT/DT_Talent.DT_Talent"));
	}
}

void UTalentManager::SetTalentDataTable(UDataTable* InDataTable)
{
	TalentDataTable = InDataTable;
}

FTalentNode UTalentManager::GetTalentNode(FName TalentName) const
{
	if (TalentDataTable)
	{
		const FTalentNode* TalentNode = TalentDataTable->FindRow<FTalentNode>(TalentName, TEXT("GetTalentNode"));
		if (TalentNode)
		{
			return *TalentNode;
		}
	}
	return FTalentNode();
}

FTalentNode UTalentManager::GetTalentNodeByID(int32 TalentID) const
{
	if (TalentDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(TalentDataTable->GetRowMap()); It; ++It)
		{
			const FTalentNode* TalentNode = reinterpret_cast<const FTalentNode*>(It.Value());
			if (TalentNode && TalentNode->TalentID == TalentID)
			{
				return *TalentNode;
			}
		}
	}
	return FTalentNode();
}

TArray<FTalentNode> UTalentManager::GetTalentsByType(ETalentType TalentType) const
{
	TArray<FTalentNode> Result;

	if (TalentDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(TalentDataTable->GetRowMap()); It; ++It)
		{
			const FTalentNode* TalentNode = reinterpret_cast<const FTalentNode*>(It.Value());
			if (TalentNode && TalentNode->TalentType == TalentType)
			{
				Result.Add(*TalentNode);
			}
		}
	}

	return Result;
}

TArray<FTalentNode> UTalentManager::GetAllTalents() const
{
	TArray<FTalentNode> Result;

	if (TalentDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(TalentDataTable->GetRowMap()); It; ++It)
		{
			const FTalentNode* TalentNode = reinterpret_cast<const FTalentNode*>(It.Value());
			if (TalentNode)
			{
				Result.Add(*TalentNode);
			}
		}
	}

	return Result;
}

bool UTalentManager::HasTalent(FName TalentName) const
{
	FTalentNode TalentNode = GetTalentNode(TalentName);
	return TalentNode.TalentID > 0;
}

bool UTalentManager::HasTalentByID(int32 TalentID) const
{
	FTalentNode TalentNode = GetTalentNodeByID(TalentID);
	return TalentNode.TalentID > 0;
}

TArray<FTalentNode> UTalentManager::GetAvailableTalents(int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs) const
{
	TArray<FTalentNode> Result;

	if (TalentDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(TalentDataTable->GetRowMap()); It; ++It)
		{
			const FTalentNode* TalentNode = reinterpret_cast<const FTalentNode*>(It.Value());
			if (TalentNode && !UnlockedTalentIDs.Contains(TalentNode->TalentID))
			{
				// 检查等级要求
				if (CurrentLevel >= TalentNode->RequiredLevel)
				{
					// 检查前置天赋
					bool bHasPrerequisites = true;
					for (int32 PrerequisiteID : TalentNode->PrerequisiteTalentIDs)
					{
						if (!UnlockedTalentIDs.Contains(PrerequisiteID))
						{
							bHasPrerequisites = false;
							break;
						}
					}

					if (bHasPrerequisites)
					{
						Result.Add(*TalentNode);
					}
				}
			}
		}
	}

	return Result;
}

bool UTalentManager::CanUnlockTalent(int32 TalentID, int32 CurrentLevel, const TSet<int32>& UnlockedTalentIDs) const
{
	FTalentNode TalentNode = GetTalentNodeByID(TalentID);
	if (TalentNode.TalentID == 0)
	{
		return false;
	}

	// 检查是否已解锁
	if (UnlockedTalentIDs.Contains(TalentID))
	{
		return false;
	}

	// 检查等级要求
	if (CurrentLevel < TalentNode.RequiredLevel)
	{
		return false;
	}

	// 检查前置天赋
	for (int32 PrerequisiteID : TalentNode.PrerequisiteTalentIDs)
	{
		if (!UnlockedTalentIDs.Contains(PrerequisiteID))
		{
			return false;
		}
	}

	return true;
}
