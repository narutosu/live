// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "livePlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class UPathFollowingComponent;

/** 追踪成功时的委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackingSuccessDelegate, class ARoleBase*, Target);

/** 角色行为状态枚举 */
UENUM(BlueprintType)
enum class ECharacterBehaviorState : uint8
{
	Idle UMETA(DisplayName = "站立"),
	Moving UMETA(DisplayName = "移动"),
	Tracking UMETA(DisplayName = "追踪"),
	Stunned UMETA(DisplayName = "眩晕")
};

/** 状态切换时的委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChangedDelegate, ECharacterBehaviorState, NewState, ECharacterBehaviorState, OldState);

/**
 *  Player controller for a top-down perspective game.
 *  Implements point and click based controls
 */
UCLASS(abstract)
class AlivePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Component used for moving along a NavMesh path. */
	UPROPERTY(VisibleDefaultsOnly, Category = AI)
	TObjectPtr<UPathFollowingComponent> PathFollowingComponent;

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, Category="Input")
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UNiagaraSystem> FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SetDestinationClickAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SetDestinationRightClickAction;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> AutoAttackAction;

	/** Skill 1 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill1Action;

	/** Skill 2 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill2Action;

	/** Skill 3 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill3Action;

	/** Skill 4 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill4Action;

	/** Skill 5 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill5Action;

	/** Skill 6 Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> Skill6Action;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	/** Set to true if we're using touch input */
	uint32 bIsTouch : 1;

	/** Saved location of the character movement destination */
	FVector CachedDestination;

	/** Time that the click input has been pressed */
	float FollowTime = 0.0f;

	/** 追踪目标 */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<class ARoleBase> TrackedTarget;
	
	/** 要拾取的物品目标 */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<class AItemActor> ItemToPickUp;
	
	FName TrackingSuccessToCast = FName("");

	/** 追踪成功的距离阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float TrackingSuccessDistance = 100.0f;

	/** 追踪更新间隔（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float TrackingUpdateInterval = 0.1f;

	/** 追踪定时器句柄 */
	FTimerHandle TrackingTimerHandle;
	
	/** 追踪目标死亡委托句柄 */
	FDelegateHandle TrackedTargetDeathDelegateHandle;

	/** 是否正在追踪 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsTracking = false;

	/** 追踪成功时的委托 */
	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnTrackingSuccessDelegate OnTrackingSuccessDelegate;

	/** 当前角色行为状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	ECharacterBehaviorState CurrentState = ECharacterBehaviorState::Idle;

	/** 状态切换时的委托 */
	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnStateChangedDelegate OnStateChangedDelegate;

	/** 眩晕定时器句柄 */
	FTimerHandle StunTimerHandle;

public:

	/** Constructor */
	AlivePlayerController();

protected:

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;
	
	/** Input handlers */
	void OnSetDestinationTriggered();
	/** Right click input handler */
	void OnSetDestinationRightClickTriggered();
	/** Auto Attack input handler */
	void OnAutoAttackTriggered();
	/** Skill 1 input handler */
	void OnSkill1Triggered();
	/** Skill 2 input handler */
	void OnSkill2Triggered();
	/** Skill 3 input handler */
	void OnSkill3Triggered();
	/** Skill 4 input handler */
	void OnSkill4Triggered();
	/** Skill 5 input handler */
	void OnSkill5Triggered();
	/** Skill 6 input handler */
	void OnSkill6Triggered();
	/** 追踪更新回调 */
	void UpdateTracking();

	/** 拾取物品更新回调 */
	void UpdateItemPickup();

	/** Helper function to cast skill by index */
	UFUNCTION(BlueprintCallable)
	void CastSkillByIndex(int32 SkillIndex);

	/** 设置角色行为状态 */
	void SetCharacterState(ECharacterBehaviorState NewState);

public:
	/** 新增：寻找范围内最近的敌人 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	class AEnemyBase* FindNearestEnemyInRange(float SearchRadius = 1000.0f, const FVector& SearchOrigin = FVector::ZeroVector);

	/** 新增：开始追踪指定的 RoleBase */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartTracking(class ARoleBase* Target, float SuccessDistance = 100.0f);

	/** 新增：取消追踪 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopTracking();

	/** 新增：获取当前追踪的目标 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	class ARoleBase* GetTrackedTarget() const { return TrackedTarget; }

	/** 新增：检查是否正在追踪 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsTracking() const { return bIsTracking; }

	/** 新增：切换到站立状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetIdleState();

	/** 新增：切换到移动状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetMovingState(const FVector& Destination);

	/** 新增：切换到追踪状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTrackingState(class ARoleBase* Target, float SuccessDistance = 100.0f,FName ParamTrackingSuccessToCast = FName(""));

	/** 新增：切换到眩晕状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetStunnedState(float Duration);

	/** 新增：获取当前状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	ECharacterBehaviorState GetCurrentState() const { return CurrentState; }

	/** 新增：检查是否处于眩晕状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsStunned() const { return CurrentState == ECharacterBehaviorState::Stunned; }

	/** 新增：检查是否处于站立状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsIdle() const { return CurrentState == ECharacterBehaviorState::Idle; }

	/** 新增：检查是否处于移动状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsMoving() const { return CurrentState == ECharacterBehaviorState::Moving; }

	/** 新增：检查是否处于追踪状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsTrackingState() const { return CurrentState == ECharacterBehaviorState::Tracking; }

	/** 新增：触发追踪成功技能 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void CastTrackingSuccessSkill();

	/** 新增：停止追踪成功技能 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopTrackingSuccessSkill();
	
	/** 新增：追踪目标死亡时的回调 */
	UFUNCTION()
	void OnTrackedTargetDeath();
};