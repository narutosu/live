#include "GSRoleAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "live.h"
#include "Net/UnrealNetwork.h"
#include "Role/RoleBase.h"

UGSRoleAttributeSet::UGSRoleAttributeSet()
	:AttackSpeed(1.0f)
{
	
}

void UGSRoleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UGSRoleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	// Compute the delta between old and new, if it is available
	float DeltaValue = 0;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		// If this was additive, store the raw delta value to be passed along later
		DeltaValue = Data.EvaluatedData.Magnitude;
	}

	
	// Get the Target actor, which should be our owner
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	ARoleBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<ARoleBase>(TargetActor);
	}
	
	if (Data.EvaluatedData.Attribute == GetSpeedAttribute())
	{
		if (TargetCharacter)
		{
			// Call for all movespeed changes
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue = GetSpeed();
			ChangeData.OldValue = 0.0f;
			ChangeData.GEModData = &Data;
			TargetCharacter->HandleMoveSpeedChanged(ChangeData);
		}
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		if (TargetCharacter)
		{
			// Call for all mana changes
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue = GetMana();
			ChangeData.OldValue = 0.0f;
			ChangeData.GEModData = &Data;
			TargetCharacter->HandleManaChanged(ChangeData);
		}
	}

	// Handle Damage attribute changes - reduce HP by the damage amount
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (TargetCharacter)
		{
			float DamageAmount = Data.EvaluatedData.Magnitude;
			if (DamageAmount > 0.f)
			{
				// Apply damage to HP
				float CurrentHP = GetHP();
				float NewHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, GetMaxHP());
				SetHP(NewHP);

				// Notify about damage taken (non-critical)
				FOnAttributeChangeData ChangeData;
				ChangeData.NewValue = NewHP;
				ChangeData.OldValue = CurrentHP;
				ChangeData.GEModData = &Data;
				TargetCharacter->HandleHealthChanged(ChangeData, false); // Not a critical hit
			}
		}
	}

	// Handle CriticalDamageValue attribute changes - reduce HP by the critical damage amount
	if (Data.EvaluatedData.Attribute == GetCriticalDamageValueAttribute())
	{
		if (TargetCharacter)
		{
			float CriticalDamageAmount = Data.EvaluatedData.Magnitude;
			if (CriticalDamageAmount > 0.f)
			{
				// Apply critical damage to HP
				float CurrentHP = GetHP();
				float NewHP = FMath::Clamp(CurrentHP - CriticalDamageAmount, 0.0f, GetMaxHP());
				SetHP(NewHP);

				// Notify about critical damage taken
				FOnAttributeChangeData ChangeData;
				ChangeData.NewValue = NewHP;
				ChangeData.OldValue = CurrentHP;
				ChangeData.GEModData = &Data;
				TargetCharacter->HandleHealthChanged(ChangeData, true); // Is a critical hit
			}
		}
	}
}

void UGSRoleAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, HP, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, HPRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, CriticalProb, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Resistance, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Experience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, ExperienceToLevelUp, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Speed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Gold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSRoleAttributeSet, CriticalDamageValue, COND_None, REPNOTIFY_Always);
}

void UGSRoleAttributeSet::OnRep_HP(const FGameplayAttributeData& OldHP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, HP, OldHP);
}

void UGSRoleAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, MaxHP, OldMaxHP);
}

void UGSRoleAttributeSet::OnRep_HPRegenRate(const FGameplayAttributeData& OldHPRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, HPRegenRate, OldHPRegenRate);
}

void UGSRoleAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Mana, OldMana);
}

void UGSRoleAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, MaxMana, OldMaxMana);
}

void UGSRoleAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldAttack)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Attack, OldAttack);
}

void UGSRoleAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UGSRoleAttributeSet::OnRep_CriticalProb(const FGameplayAttributeData& OldCriticalProb)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, CriticalProb, OldCriticalProb);
}

void UGSRoleAttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, CriticalDamage, OldCriticalDamage);
}

void UGSRoleAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Armor, OldArmor);
}

void UGSRoleAttributeSet::OnRep_Resistance(const FGameplayAttributeData& OldResistance)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Resistance, OldResistance);
}

void UGSRoleAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Level, OldLevel);
}

void UGSRoleAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Experience, OldExperience);
}

void UGSRoleAttributeSet::OnRep_ExperienceToLevelUp(const FGameplayAttributeData& OldExperienceToLevelUp)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, ExperienceToLevelUp, OldExperienceToLevelUp);
}

void UGSRoleAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Speed, OldSpeed);
	UE_LOG(Loglive, Warning, TEXT("OnRep_Speed"));
}

void UGSRoleAttributeSet::OnRep_Gold(const FGameplayAttributeData& OldGold)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Gold, OldGold);
}

void UGSRoleAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, Damage, OldDamage);
}

void UGSRoleAttributeSet::OnRep_CriticalDamageValue(const FGameplayAttributeData& OldCriticalDamageValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSRoleAttributeSet, CriticalDamageValue, OldCriticalDamageValue);
}
