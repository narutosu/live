// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GAS/Attribute/GSRoleAttributeSet.h"
#include "Skill/SkillComponent.h"
#include "Item/InventoryComponent.h"
#include "RoleBase.generated.h"


// struct FGameplayAbilitySpecHandle;
UCLASS()
class LIVE_API ARoleBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARoleBase();

	// Delegate for level changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32, NewLevel, int32, OldLevel);
	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnLevelChanged OnLevelChanged;
	
	// 新增：血量变化 Delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedDelegate, float, NewHealth, float, OldHealth, bool, bIsCriticalHit);
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnHealthChangedDelegate OnHealthChangedDelegate;
	
	// 新增：经验变化 Delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceChangedDelegate, float, NewExperience, float, OldExperience);
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnExperienceChangedDelegate OnExperienceChangedDelegate;
	
	// 新增：死亡 Delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnDeathDelegate OnDeathDelegate;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Implement IAbilitySystemInterface
	class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UGSRoleAttributeSet* GetRoleAttributeSet() const;
protected:
	UPROPERTY()
	class URPGAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UGSRoleAttributeSet* RoleAttributeSet;

	/** The level of this character, should not be modified directly once it has already spawned */
	UPROPERTY(EditAnywhere, Replicated, Category = Abilities)
	int32 CharacterLevel;

	UPROPERTY()
	int32 bAbilitiesInitialized;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns the character level that is passed to the ability system */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetCharacterLevel() const;

	/** Modifies the character level, this may change abilities. Returns true on success */
	UFUNCTION(BlueprintCallable)
	virtual bool SetCharacterLevel(int32 NewLevel);

	UFUNCTION(BlueprintCallable)
    void StopMove();

public:
	//属性获取
	/** Returns current health, will be 0 if dead */
	UFUNCTION(BlueprintCallable)
	virtual float GetHealth() const;

	/** Returns maximum health, health will never be greater than this */
	UFUNCTION(BlueprintCallable)
	virtual float GetMaxHealth() const;

	/** Returns current mana */
	UFUNCTION(BlueprintCallable)
	virtual float GetMana() const;

	/** Returns maximum mana, mana will never be greater than this */
	UFUNCTION(BlueprintCallable)
	virtual float GetMaxMana() const;

	/** Returns current movement speed */
	UFUNCTION(BlueprintCallable)
	virtual float GetMoveSpeed() const;

	/** Returns current experience */
	UFUNCTION(BlueprintCallable)
	virtual float GetExperience() const;

	/** Returns experience needed to level up */
	UFUNCTION(BlueprintCallable)
	virtual float GetExperienceToLevelUp() const;

	/** Sets experience needed to level up */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void SetExperienceToLevelUp(float NewExperienceToLevelUp);
	
	//属性回调 - 内部使用
	virtual void HandleHealthChanged(const FOnAttributeChangeData& Data, bool bIsCriticalHit = false);
	virtual void HandleManaChanged(const FOnAttributeChangeData& Data);
	virtual void HandleMoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void HandleExperienceChanged(const FOnAttributeChangeData& Data);

public:
	//蓝图可实现的属性变化事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnHealthChanged(float NewValue, float OldValue, bool bIsCriticalHit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnManaChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnMoveSpeedChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnExperienceChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnLevelUp(int32 NewLevel, int32 OldLevel);

public:
	UFUNCTION(BlueprintCallable)
	virtual float GetAttackRate() const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	USkillComponent* GetSkillComponent() const { return SkillComponent; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	class USkillComponent* SkillComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	class UInventoryComponent* InventoryComponent;
	

	/** Death montage to play when character dies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;
	
	/** Passive gameplay effects applied on creation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Abilities)
	TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

	// 新增：应用被动 GameplayEffects
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	virtual void ApplyPassiveGameplayEffects();
	
	/** Apply the startup gameplay abilities and effects */
	void AddStartupGameplayAbilities();

	/** Attempts to remove any startup gameplay abilities */
	void RemoveStartupGameplayAbilities();

	/** Handle character death */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void Death();
	
	/** Called when death montage ends */
	UFUNCTION()
	virtual void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Add experience and handle level up */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void AddExperience(float ExperienceAmount);

	/** Handle level up logic */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void HandleLevelUp();
};