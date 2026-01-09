// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SkillData.h"

FSkillLevelData FSkillData::GetLevelData(int32 Level) const
{
	for (const FSkillLevelData& Data : LevelData)
	{
		if (Data.Level == Level)
		{
			return Data;
		}
	}
	return FSkillLevelData();
}
