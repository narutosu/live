// Fill out your copyright notice in the Description page of Project Settings.

#include "Talent/TalentComponent.h"
#include "Role/RoleBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UTalentComponent::UTalentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTalentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner()))
	{
		RoleOwner->OnLevelChanged.AddDynamic(this, &UTalentComponent::OnOwnerLevelChanged);
	}
}

bool UTalentComponent::UnlockTalent(int32 TalentID)
{
	if (!TalentDataAsset.IsValid())
	{
		return false;
	}

	UTalentData* TalentData = TalentDataAsset.LoadSynchronous();
	if (!TalentData)
	{
		return false;
	}

	if (UnlockedTalentIDs.Contains(TalentID))
	{
		return false;
	}

	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner)
	{
		return false;
	}

	if (!TalentData->CanUnlockTalent(TalentID, RoleOwner->GetCharacterLevel(), UnlockedTalentIDs))
	{
		return false;
	}

	FTalentNode TalentNode = TalentData->GetTalentNode(TalentID);
	if (TalentNode.TalentID == 0)
	{
		return false;
	}

	UnlockedTalentIDs.Add(TalentID);
	TalentRanks.Add(TalentID, 1);

	ApplyTalentEffect(TalentNode);

	return true;
}

TArray<FTalentNode> UTalentComponent::GetAvailableTalents()
{
	TArray<FTalentNode> AvailableTalents;

	if (!TalentDataAsset.IsValid())
	{
		return AvailableTalents;
	}

	UTalentData* TalentData = TalentDataAsset.LoadSynchronous();
	if (!TalentData)
	{
		return AvailableTalents;
	}

	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner)
	{
		return AvailableTalents;
	}

	return TalentData->GetAvailableTalents(RoleOwner->GetCharacterLevel(), UnlockedTalentIDs);
}

TArray<FTalentNode> UTalentComponent::GetUnlockedTalents()
{
	TArray<FTalentNode> UnlockedTalents;

	if (!TalentDataAsset.IsValid())
	{
		return UnlockedTalents;
	}

	UTalentData* TalentData = TalentDataAsset.LoadSynchronous();
	if (!TalentData)
	{
		return UnlockedTalents;
	}

	for (int32 TalentID : UnlockedTalentIDs)
	{
		FTalentNode Node = TalentData->GetTalentNode(TalentID);
		if (Node.TalentID != 0)
		{
			UnlockedTalents.Add(Node);
		}
	}

	return UnlockedTalents;
}

void UTalentComponent::OnOwnerLevelChanged(int32 NewLevel, int32 OldLevel)
{
}

void UTalentComponent::ApplyTalentEffect(const FTalentNode& TalentNode)
{
	ARoleBase* RoleOwner = Cast<ARoleBase>(GetOwner());
	if (!RoleOwner || !RoleOwner->GetAbilitySystemComponent())
	{
		return;
	}

	if (TalentNode.GameplayEffectClass.IsValid())
	{
		UAbilitySystemComponent* ASC = RoleOwner->GetAbilitySystemComponent();
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TalentNode.GameplayEffectClass.LoadSynchronous(), RoleOwner->GetCharacterLevel(), ASC->MakeEffectContext());
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		}
	}
}

void UTalentComponent::RemoveTalentEffect(const FTalentNode& TalentNode)
{
}

void UTalentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}