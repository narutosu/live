// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SkillManager.h"

USkillManager* USkillManager::Instance = nullptr;

USkillManager* USkillManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<USkillManager>();
		Instance->AddToRoot();
		Instance->Initialize();
	}
	return Instance;
}

void USkillManager::Initialize()
{
	if (!SkillDataTable)
	{
		SkillDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Script/Engine.DataTable'/Game/DT/DT_Skill.DT_Skill'"));
	}
}

void USkillManager::SetSkillDataTable(UDataTable* InDataTable)
{
	SkillDataTable = InDataTable;
}

FSkillData USkillManager::GetSkillData(FName SkillName) const
{
	if (SkillDataTable)
	{
		const FSkillData* SkillData = SkillDataTable->FindRow<FSkillData>(SkillName, TEXT("GetSkillData"));
		if (SkillData)
		{
			return *SkillData;
		}
	}
	return FSkillData();
}

FSkillData USkillManager::GetSkillDataByID(int32 SkillID) const
{
	if (SkillDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(SkillDataTable->GetRowMap()); It; ++It)
		{
			const FSkillData* SkillData = reinterpret_cast<const FSkillData*>(It.Value());
			if (SkillData && SkillData->SkillID == SkillID)
			{
				return *SkillData;
			}
		}
	}
	return FSkillData();
}

TArray<FSkillData> USkillManager::GetSkillsByType(ESkillType SkillType) const
{
	TArray<FSkillData> Result;

	if (SkillDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(SkillDataTable->GetRowMap()); It; ++It)
		{
			const FSkillData* SkillData = reinterpret_cast<const FSkillData*>(It.Value());
			if (SkillData && SkillData->SkillType == SkillType)
			{
				Result.Add(*SkillData);
			}
		}
	}

	return Result;
}

TArray<FSkillData> USkillManager::GetAllSkills() const
{
	TArray<FSkillData> Result;

	if (SkillDataTable)
	{
		for (TMap<FName, uint8*>::TConstIterator It(SkillDataTable->GetRowMap()); It; ++It)
		{
			const FSkillData* SkillData = reinterpret_cast<const FSkillData*>(It.Value());
			if (SkillData)
			{
				Result.Add(*SkillData);
			}
		}
	}

	return Result;
}

bool USkillManager::HasSkill(FName SkillName) const
{
	FSkillData SkillData = GetSkillData(SkillName);
	return SkillData.SkillID > 0;
}

bool USkillManager::HasSkillByID(int32 SkillID) const
{
	FSkillData SkillData = GetSkillDataByID(SkillID);
	return SkillData.SkillID > 0;
}