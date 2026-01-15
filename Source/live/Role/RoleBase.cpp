// Fill out your copyright notice in the Description page of Project Settings.

#include "Role/RoleBase.h"
#include "GAS/Common/RPGAbilitySystemComponent.h"
#include "GAS/Attribute/GSRoleAttributeSet.h"
#include "GAS/Common/RPGGameplayAbility.h"
#include "Skill/SkillComponent.h"
#include "Item/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"


// Sets default values
ARoleBase::ARoleBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Create the attribute set, this replicates by default
	RoleAttributeSet = CreateDefaultSubobject<UGSRoleAttributeSet>(TEXT("RoleAttributeSet"));

	SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	CharacterLevel = 1;
	bAbilitiesInitialized = false;
}

void ARoleBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//初始化角色技能
	if (GetLocalRole() == ROLE_Authority && !bAbilitiesInitialized)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetManaAttribute()).AddUObject(this, &ARoleBase::HandleManaChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetSpeedAttribute()).AddUObject(this, &ARoleBase::HandleMoveSpeedChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetExperienceAttribute()).AddUObject(this, &ARoleBase::HandleExperienceChanged);
		AddStartupGameplayAbilities();
		bAbilitiesInitialized = true;
	}
}

void ARoleBase::UnPossessed()
{
	
}

void ARoleBase::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void ARoleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoleBase, CharacterLevel);
}

UAbilitySystemComponent* ARoleBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSRoleAttributeSet* ARoleBase::GetRoleAttributeSet() const
{
	return RoleAttributeSet;
}

// Called when the game starts or when spawned
void ARoleBase::BeginPlay()
{
	Super::BeginPlay();
	SkillComponent->LearnSkillByName(FName("DropExp"));
	SkillComponent->LearnSkillByName(FName("LevelUp"));
	
	if (AbilitySystemComponent && RoleAttributeSet)
	{
		// Bind attribute change delegates
		// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetHPAttribute()).AddUObject(this, &ARoleBase::HandleHealthChanged);

		// Manually trigger attribute change handlers to initialize values
		// FOnAttributeChangeData ManaData;
		// ManaData.Attribute = RoleAttributeSet->GetManaAttribute();
		// ManaData.NewValue = RoleAttributeSet->GetMana();
		// ManaData.OldValue = RoleAttributeSet->GetMana();
		// HandleManaChanged(ManaData);
		//
		// FOnAttributeChangeData SpeedData;
		// SpeedData.Attribute = RoleAttributeSet->GetSpeedAttribute();
		// SpeedData.NewValue = RoleAttributeSet->GetSpeed();
		// SpeedData.OldValue = RoleAttributeSet->GetSpeed();
		// HandleMoveSpeedChanged(SpeedData);
		//
		// FOnAttributeChangeData ExperienceData;
		// ExperienceData.Attribute = RoleAttributeSet->GetExperienceAttribute();
		// ExperienceData.NewValue = RoleAttributeSet->GetExperience();
		// ExperienceData.OldValue = RoleAttributeSet->GetExperience();
		// HandleExperienceChanged(ExperienceData);
	}
}

// Called every frame
void ARoleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle smooth rotation to target location
	if (bIsRotatingToTarget)
	{
		// Calculate direction to target
		FVector ActorLocation = GetActorLocation();
		FVector Direction = TargetRotationLocation - ActorLocation;
		Direction.Z = 0.0f; // Keep rotation horizontal

		if (Direction.IsNearlyZero())
		{
			bIsRotatingToTarget = false;
			return;
		}

		// Calculate target rotation
		FRotator TargetRotation = Direction.Rotation();
		FRotator CurrentRotation = GetActorRotation();

		// Calculate angle difference
		float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

		// Check if within tolerance
		if (AngleDifference <= RotationToleranceAngle)
		{
			bIsRotatingToTarget = false;
			return;
		}

		// Interpolate rotation
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CurrentRotationSpeed);
		SetActorRotation(NewRotation);
	}
}

// Called to bind functionality to input
void ARoleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

int32 ARoleBase::GetCharacterLevel() const
{
	return CharacterLevel;
}

bool ARoleBase::SetCharacterLevel(int32 NewLevel)
{
	if (CharacterLevel != NewLevel && NewLevel > 0)
	{
		int32 OldLevel = CharacterLevel;
		RemoveStartupGameplayAbilities();
		CharacterLevel = NewLevel;
		AddStartupGameplayAbilities();
		OnLevelChanged.Broadcast(NewLevel, OldLevel);
		return true;
	}
	return false;
}

void ARoleBase::StopMove()
{
	GetMovementComponent()->StopMovementImmediately();
}

float ARoleBase::GetHealth() const
{
	return RoleAttributeSet->GetHP();
}

float ARoleBase::GetMaxHealth() const
{
	return RoleAttributeSet->GetMaxHP();
}

float ARoleBase::GetMana() const
{
	return RoleAttributeSet->GetMana();
}

float ARoleBase::GetMaxMana() const
{
	return RoleAttributeSet->GetMaxMana();
}

float ARoleBase::GetMoveSpeed() const
{
	return RoleAttributeSet->GetSpeed();
}

float ARoleBase::GetExperience() const
{
	return RoleAttributeSet->GetExperience();
}

void ARoleBase::HandleHealthChanged(const FOnAttributeChangeData& Data, bool bIsCriticalHit)
{
	// We only call the BP callback if this is not the initial ability setup
	if (bAbilitiesInitialized)
	{
		OnHealthChanged(Data.NewValue, Data.OldValue, bIsCriticalHit);
		OnHealthChangedDelegate.Broadcast(Data.NewValue, Data.OldValue, bIsCriticalHit);
		
		// Check if character died
		if (Data.NewValue <= 0.0f && Data.OldValue > 0.0f)
		{
			Death();
		}
	}
}

void ARoleBase::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	OnManaChanged(Data.NewValue, Data.OldValue);
}

void ARoleBase::HandleMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	// Update the character movement's walk speed
	GetCharacterMovement()->MaxWalkSpeed = RoleAttributeSet->GetSpeed();
	OnMoveSpeedChanged(Data.NewValue, Data.OldValue);
}

void ARoleBase::HandleExperienceChanged(const FOnAttributeChangeData& Data)
{
	OnExperienceChanged(Data.NewValue, Data.OldValue);
	OnExperienceChangedDelegate.Broadcast(Data.NewValue, Data.OldValue);
		
	float ExperienceNeeded = RoleAttributeSet->GetExperienceToLevelUp();
	
	// If ExperienceNeeded is 0 or negative, use default value of 100
	if (ExperienceNeeded <= 0.0f)
	{
		ExperienceNeeded = 100.0f;
	}
	if (Data.NewValue >= ExperienceNeeded)
	{
		HandleLevelUp();
	}
}

float ARoleBase::GetAttackRate() const
{
	return RoleAttributeSet->GetAttackSpeed();
}

void ARoleBase::ApplyPassiveGameplayEffects()
{
	for (TSubclassOf<UGameplayEffect>& GameplayEffect : PassiveGameplayEffects)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, GetCharacterLevel(), EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent);
		}
	}
}

void ARoleBase::AddStartupGameplayAbilities()
{
	if (GetLocalRole() == ROLE_Authority && !bAbilitiesInitialized)
	{
		ApplyPassiveGameplayEffects();
		bAbilitiesInitialized = true;
	}
}

void ARoleBase::RemoveStartupGameplayAbilities()
{
	if (GetLocalRole() == ROLE_Authority && bAbilitiesInitialized)
	{
		FGameplayEffectQuery Query;
		Query.EffectSource = this;
		AbilitySystemComponent->RemoveActiveEffects(Query);
		bAbilitiesInitialized = false;
	}
}

void ARoleBase::Death()
{
	SkillComponent->CastSkillByName(FName("DropExp"));
	// Broadcast death delegate
	OnDeathDelegate.Broadcast();
	
	// Stop all movement
	StopMove();
	
	// Cancel all abilities
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}
	
	// Disable collision
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// Play death montage if specified
	if (DeathMontage)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ARoleBase::OnDeathMontageEnded);
		PlayAnimMontage(DeathMontage, 1.0f, NAME_None);
		GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(MontageEndedDelegate, DeathMontage);
	}
	else
	{
		// If no montage, destroy immediately
		Destroy();
	}
}

void ARoleBase::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Destroy the character when death montage ends
	Destroy();
}

void ARoleBase::AddExperience(float ExperienceAmount)
{
	if (RoleAttributeSet && ExperienceAmount > 0.0f)
	{
		float CurrentExperience = RoleAttributeSet->GetExperience();
		float NewExperience = CurrentExperience + ExperienceAmount;
		RoleAttributeSet->SetExperience(NewExperience);
	}
}

float ARoleBase::GetExperienceToLevelUp() const
{
	if (RoleAttributeSet)
	{
		return RoleAttributeSet->GetExperienceToLevelUp();
	}
	return 100.0f; // Default value
}

void ARoleBase::SetExperienceToLevelUp(float NewExperienceToLevelUp)
{
	if (RoleAttributeSet && NewExperienceToLevelUp > 0.0f)
	{
		RoleAttributeSet->SetExperienceToLevelUp(NewExperienceToLevelUp);
	}
}

void ARoleBase::HandleLevelUp()
{
	if (!RoleAttributeSet)
	{
		return;
	}

	float CurrentExperience = RoleAttributeSet->GetExperience();
	float ExperienceNeeded = RoleAttributeSet->GetExperienceToLevelUp();
	
	// If ExperienceNeeded is 0 or negative, use default value of 100
	if (ExperienceNeeded <= 0.0f)
	{
		ExperienceNeeded = 100.0f;
	}
	
	int32 CurrentLevel = FMath::TruncToInt(RoleAttributeSet->GetLevel());
	int32 OldLevel = CurrentLevel;
	
	// Calculate how many levels to level up (support continuous level up)
	int32 LevelsToGain = 1;
	
	if (LevelsToGain > 0)
	{
		// Calculate new level
		int32 NewLevel = CurrentLevel + LevelsToGain;
		// Broadcast level up event
		OnLevelUp(NewLevel, OldLevel);
		OnLevelChanged.Broadcast(NewLevel, OldLevel);
		SkillComponent->CastSkillByName(FName("LevelUp"));
		// Update character level
		CharacterLevel = NewLevel;
		// Update level in attribute set
		RoleAttributeSet->SetLevel(NewLevel);
		
		// Re-apply abilities for new level
		RemoveStartupGameplayAbilities();
		AddStartupGameplayAbilities();
		
		// 这里可能再次触发升级
		float RemainingExperience = CurrentExperience - (LevelsToGain * ExperienceNeeded);
		RoleAttributeSet->SetExperience(RemainingExperience);
	}
}

void ARoleBase::SmoothRotateToLocation(const FVector& TargetLocation)
{
	// Update target location and parameters (this will override any previous rotation)
	TargetRotationLocation = TargetLocation;
	bIsRotatingToTarget = true;
}