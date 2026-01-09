// Fill out your copyright notice in the Description page of Project Settings.

#include "Role/RoleBase.h"
#include "GAS/Common/RPGAbilitySystemComponent.h"
#include "GAS/Attribute/GSRoleAttributeSet.h"
#include "GAS/Common/RPGGameplayAbility.h"
#include "Skill/SkillComponent.h"
#include "Item/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


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
	
	if (AbilitySystemComponent && RoleAttributeSet)
	{
		// Bind attribute change delegates
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetHPAttribute()).AddUObject(this, &ARoleBase::HandleHealthChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetManaAttribute()).AddUObject(this, &ARoleBase::HandleManaChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RoleAttributeSet->GetSpeedAttribute()).AddUObject(this, &ARoleBase::HandleMoveSpeedChanged);
		
		bAbilitiesInitialized = true;
	}
}

// Called every frame
void ARoleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		// Our level changed so we need to refresh abilities
		// RemoveStartupGameplayAbilities();
		CharacterLevel = NewLevel;
		// AddStartupGameplayAbilities();

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

void ARoleBase::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	// We only call the BP callback if this is not the initial ability setup
	if (bAbilitiesInitialized)
	{
		OnHealthChanged(Data.NewValue, Data.OldValue);
	}
}

void ARoleBase::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	if (bAbilitiesInitialized)
	{
		OnManaChanged(Data.NewValue, Data.OldValue);
	}
}

void ARoleBase::HandleMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	// Update the character movement's walk speed
	GetCharacterMovement()->MaxWalkSpeed = RoleAttributeSet->GetSpeed();
	
	// Call blueprint event
	if (bAbilitiesInitialized)
	{
		OnMoveSpeedChanged(Data.NewValue, Data.OldValue);
	}
}

float ARoleBase::GetAttackRate() const
{
	return RoleAttributeSet->GetAttackSpeed();
}