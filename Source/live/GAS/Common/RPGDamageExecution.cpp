// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGDamageExecution.h"
#include "GAS/Attribute/GSRoleAttributeSet.h"
#include "AbilitySystemComponent.h"

struct RPGDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamageValue);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalProb);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);

	RPGDamageStatics()
	{
		// Capture the Target's Damage attribute. Do not snapshot it.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, Damage, Target, false);

		// Capture the Target's CriticalDamageValue attribute. Do not snapshot it.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, CriticalDamageValue, Target, false);

		// Capture the Target's Armor attribute. Do not snapshot it, because we want to use the armor value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, Armor, Target, false);

		// Capture the Source's Attack. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		// (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the Attack at the moment
		// the projectile was launched, not when it hits).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, Attack, Source, true);

		// Capture the Source's CriticalProb. Snapshot it to use the value at the moment the effect is created.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, CriticalProb, Source, true);

		// Capture the Source's CriticalDamage. Snapshot it to use the value at the moment the effect is created.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSRoleAttributeSet, CriticalDamage, Source, true);
	}
};

static const RPGDamageStatics& DamageStatics()
{
	static RPGDamageStatics DmgStatics;
	return DmgStatics;
}

URPGDamageExecution::URPGDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageValueDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalProbDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageDef);
}

void URPGDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// --------------------------------------
	//	Damage Done = Attack * (100 / (100 + Armor)) * CriticalMultiplier
	//	CriticalMultiplier = 1 + CriticalDamage if critical hit occurs
	// --------------------------------------

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);

	float Attack = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDef, EvaluationParameters, Attack);

	float CriticalProb = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalProbDef, EvaluationParameters, CriticalProb);

	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalDamageDef, EvaluationParameters, CriticalDamage);

	// Calculate damage reduction based on armor
	float DamageReduction = 100.0f / (100.0f + Armor);
	float DamageDone = Attack * DamageReduction;

	// Check for critical hit
	bool bIsCriticalHit = false;
	if (CriticalProb > 0.f)
	{
		// CriticalProb is a percentage (0-100), convert to 0-1 range for random check
		float CriticalChance = CriticalProb / 100.0f;
		bIsCriticalHit = FMath::FRand() < CriticalChance;
	}

	// Apply critical damage multiplier if critical hit
	if (bIsCriticalHit)
	{
		// CriticalDamage is a percentage (e.g., 50 means 50% extra damage)
		float CriticalMultiplier = 1.0f + (CriticalDamage / 100.0f);
		DamageDone *= CriticalMultiplier;
	}

	// Apply damage to the appropriate attribute based on whether it's a critical hit
	if (DamageDone > 0.f)
	{
		if (bIsCriticalHit)
		{
			// Apply to CriticalDamageValue attribute (will trigger HP reduction in AttributeSet)
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().CriticalDamageValueProperty, EGameplayModOp::Additive, DamageDone));
		}
		else
		{
			// Apply to Damage attribute (will trigger HP reduction in AttributeSet)
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
		}
	}
}