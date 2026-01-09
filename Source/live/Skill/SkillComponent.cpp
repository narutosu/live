// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SkillComponent.h"
#include "Skill/SkillManager.h"
#include "Role/RoleBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/Common/RPGGameplayAbility.h"
#include "Net/UnrealNetwork.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner()))
	{
		RoleOwner->OnLevelChanged.AddDynamic(this, &USkillComponent::OnOwnerLevelChanged);
	}
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (FActiveSkill& ActiveSkill : ActiveSkills)
	{
		if (ActiveSkill.CurrentCooldown > 0.0f)
		{
			float OldCooldown = ActiveSkill.CurrentCooldown;
			ActiveSkill.CurrentCooldown -= DeltaTime;
			if (ActiveSkill.CurrentCooldown < 0.0f)
			{
				ActiveSkill.CurrentCooldown = 0.0f;
			}
			if (OldCooldown > 0.0f && ActiveSkill.CurrentCooldown == 0.0f)
			{
				OnSkillCooldownChanged.Broadcast(ActiveSkill.SkillID);
			}
		}
	}
}

bool USkillComponent::LearnSkill(int32 SkillID)
{
	if (HasSkill(SkillID))
	{
		return false;
	}

	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner)
	{
		return false;
	}

	FSkillData SkillData = USkillManager::Get()->GetSkillDataByID(SkillID);
	if (SkillData.SkillID > 0 && SkillData.RequiredLevels.Num() > 0)
	{
		int32 RequiredLevel = SkillData.RequiredLevels[0];
		if (RoleOwner->GetCharacterLevel() >= RequiredLevel)
		{
			// Find the first available slot (0-5)
			int32 AvailableSlot = -1;
			for (int32 SlotIndex = 0; SlotIndex < 6; SlotIndex++)
			{
				bool bSlotOccupied = false;
				for (const FActiveSkill& ActiveSkill : ActiveSkills)
				{
					if (ActiveSkill.SlotIndex == SlotIndex)
					{
						bSlotOccupied = true;
						break;
					}
				}
				if (!bSlotOccupied)
				{
					AvailableSlot = SlotIndex;
					break;
				}
			}

			// If no available slot, don't learn the skill
			if (AvailableSlot == -1)
			{
				return false;
			}

			FActiveSkill NewSkill;
			NewSkill.SkillID = SkillID;
			NewSkill.CurrentLevel = 1;
			NewSkill.SlotIndex = AvailableSlot;
			ActiveSkills.Add(NewSkill);

			GrantAbility(SkillID, SkillData);

			// Automatically activate passive skills
			if (SkillData.SkillType == ESkillType::Passive)
			{
				CastSkill(SkillID);
			}
			
			OnSkillLevelChanged.Broadcast(SkillID, 1);
			return true;
		}
	}

	return false;
}

bool USkillComponent::LearnSkillByName(FName SkillName)
{
	FSkillData SkillData = USkillManager::Get()->GetSkillData(SkillName);
	if (SkillData.SkillID > 0)
	{
		return LearnSkill(SkillData.SkillID);
	}
	return false;
}

bool USkillComponent::UpgradeSkill(int32 SkillID)
{
	FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return false;
	}

	FSkillData SkillData = USkillManager::Get()->GetSkillDataByID(SkillID);
	if (SkillData.SkillID > 0)
	{
		if (ActiveSkill->CurrentLevel >= SkillData.MaxLevel)
		{
			return false;
		}

		int32 NextLevel = ActiveSkill->CurrentLevel + 1;
		if (SkillData.RequiredLevels.IsValidIndex(NextLevel - 1))
		{
			ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
			if (RoleOwner && RoleOwner->GetCharacterLevel() >= SkillData.RequiredLevels[NextLevel - 1])
			{
				ActiveSkill->CurrentLevel = NextLevel;
				OnSkillLevelChanged.Broadcast(SkillID, NextLevel);
				return true;
			}
		}
	}

	return false;
}

bool USkillComponent::CastSkill(int32 SkillID, AActor* Target, const FVector& TargetLocation)
{
	FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return false;
	}

	if (!CanCastSkill(SkillID))
	{
		return false;
	}

	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return false;
	}

	FSkillData SkillData = USkillManager::Get()->GetSkillDataByID(SkillID);
	if (SkillData.SkillID > 0 && SkillData.AbilityClass.IsValid())
	{
		FSkillLevelData LevelData = SkillData.GetLevelData(ActiveSkill->CurrentLevel);
		ActiveSkill->CurrentCooldown = LevelData.Cooldown;
		ActiveSkill->LastCastTime = GetWorld()->GetTimeSeconds();

		UAbilitySystemComponent* ASC = RoleOwner->GetAbilitySystemComponent();
		if (ActiveSkill->AbilityHandle.IsValid())
		{
			ASC->TryActivateAbility(ActiveSkill->AbilityHandle);
		}

		OnSkillCooldownChanged.Broadcast(SkillID);
		return true;
	}

	return false;
}

bool USkillComponent::CanCastSkill(int32 SkillID) const
{
	const FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return false;
	}

	if (ActiveSkill->CurrentCooldown > 0.0f)
	{
		return false;
	}

	return true;
}

float USkillComponent::GetSkillCooldown(int32 SkillID) const
{
	const FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return 0.0f;
	}

	FSkillData SkillData = USkillManager::Get()->GetSkillDataByID(SkillID);
	if (SkillData.SkillID > 0)
	{
		return SkillData.GetLevelData(ActiveSkill->CurrentLevel).Cooldown;
	}

	return 0.0f;
}

float USkillComponent::GetSkillCooldownRemaining(int32 SkillID) const
{
	const FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return 0.0f;
	}

	return ActiveSkill->CurrentCooldown;
}

int32 USkillComponent::GetSkillLevel(int32 SkillID) const
{
	const FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (!ActiveSkill)
	{
		return 0;
	}

	return ActiveSkill->CurrentLevel;
}

bool USkillComponent::HasSkill(int32 SkillID) const
{
	return FindActiveSkill(SkillID) != nullptr;
}

bool USkillComponent::CastSkillByName(FName SkillName, AActor* Target, const FVector& TargetLocation)
{
	FSkillData SkillData = USkillManager::Get()->GetSkillData(SkillName);
	if (SkillData.SkillID > 0)
	{
		return CastSkill(SkillData.SkillID, Target, TargetLocation);
	}
	return false;
}

bool USkillComponent::CastSkillBySlot(int32 SlotIndex, AActor* Target, const FVector& TargetLocation)
{
	// Check if slot index is valid (0-5)
	if (SlotIndex < 0 || SlotIndex >= 6)
	{
		return false;
	}

	// Find the skill in the specified slot
	for (FActiveSkill& ActiveSkill : ActiveSkills)
	{
		if (ActiveSkill.SlotIndex == SlotIndex)
		{
			return CastSkill(ActiveSkill.SkillID, Target, TargetLocation);
		}
	}

	// No skill found in the specified slot
	return false;
}

void USkillComponent::OnOwnerLevelChanged(int32 NewLevel, int32 OldLevel)
{
}

FActiveSkill* USkillComponent::FindActiveSkill(int32 SkillID)
{
	for (FActiveSkill& ActiveSkill : ActiveSkills)
	{
		if (ActiveSkill.SkillID == SkillID)
		{
			return &ActiveSkill;
		}
	}
	return nullptr;
}

const FActiveSkill* USkillComponent::FindActiveSkill(int32 SkillID) const
{
	for (const FActiveSkill& ActiveSkill : ActiveSkills)
	{
		if (ActiveSkill.SkillID == SkillID)
		{
			return &ActiveSkill;
		}
	}
	return nullptr;
}

void USkillComponent::GrantAbility(int32 SkillID, const FSkillData& SkillData)
{
	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return;
	}
	UClass* AbilityClass = SkillData.AbilityClass.LoadSynchronous();
	if (AbilityClass)
	{
		UAbilitySystemComponent* ASC = RoleOwner->GetAbilitySystemComponent();
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(
			FGameplayAbilitySpec(AbilityClass, 1, SkillID, RoleOwner)
		);

		FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
		if (ActiveSkill)
		{
			ActiveSkill->AbilityHandle = Handle;
		}
	}
}

void USkillComponent::RemoveAbility(int32 SkillID)
{
	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return;
	}

	FActiveSkill* ActiveSkill = FindActiveSkill(SkillID);
	if (ActiveSkill && ActiveSkill->AbilityHandle.IsValid())
	{
		RoleOwner->GetAbilitySystemComponent()->ClearAbility(ActiveSkill->AbilityHandle);
	}
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillComponent, ActiveSkills);
}