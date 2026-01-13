// Copyright Epic Games, Inc. All Rights Reserved.

#include "livePlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "liveCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "live.h"
#include "Role/EnemyBase.h"
#include "Components/SphereComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "UObject/UObjectGlobals.h"
#include "Skill/SkillManager.h"
#include "Skill/SkillData.h"
#include "GAS/Common/RPGGameplayAbility.h"

AlivePlayerController::AlivePlayerController()
{
	bIsTouch = false;
	bMoveToMouseCursor = false;

	// create the path following comp
	PathFollowingComponent = CreateDefaultSubobject<UPathFollowingComponent>(TEXT("Path Following Component"));

	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	
	// 初始化状态
	CurrentState = ECharacterBehaviorState::Idle;
}

void AlivePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Setup mouse input events
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AlivePlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AlivePlayerController::OnSetDestinationTriggered);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AlivePlayerController::OnSetDestinationReleased);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AlivePlayerController::OnSetDestinationReleased);

			// Setup touch input events
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AlivePlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AlivePlayerController::OnTouchTriggered);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AlivePlayerController::OnTouchReleased);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AlivePlayerController::OnTouchReleased);

			// Setup auto attack input event
			EnhancedInputComponent->BindAction(AutoAttackAction, ETriggerEvent::Triggered, this, &AlivePlayerController::OnAutoAttackTriggered);
		}
		else
		{
			UE_LOG(Loglive, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		}
	}
}

void AlivePlayerController::OnInputStarted()
{
	StopTracking();
	StopTrackingSuccessSkill();
	StopMovement();

	// Update the move destination to wherever the cursor is pointing at
	UpdateCachedDestination();
}

void AlivePlayerController::OnSetDestinationTriggered()
{
	StopTracking();
	StopTrackingSuccessSkill();
	
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// Update the move destination to wherever the cursor is pointing at
	UpdateCachedDestination();
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AlivePlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		StopTracking();
		StopTrackingSuccessSkill();
		
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AlivePlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AlivePlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AlivePlayerController::UpdateCachedDestination()
{
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	FHitResult HitTarget;
	bool bHitSuccessfulTarget = GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, false, HitTarget);

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
		
		// 检查是否点击到了 RoleBase（敌人或友方角色）
		if (Hit.GetActor())
		{
			ARoleBase* HitRole = Cast<ARoleBase>(Hit.GetActor());
			if (HitRole && IsValid(HitRole))
			{
				// 更新追踪目标
				TrackedTarget = HitRole;
				UE_LOG(Loglive, Log, TEXT("UpdateCachedDestination: Updated TrackedTarget to %s"), *HitRole->GetName());
			}
		}
	}
	
	if (bHitSuccessfulTarget)
    {
        AActor* Actor = HitTarget.HitObjectHandle.GetCachedActor();
		ARoleBase* HitRole = Cast<ARoleBase>(Actor);
		if (HitRole && IsValid(HitRole))
        {
            // 更新追踪目标
            TrackedTarget = HitRole;
            UE_LOG(Loglive, Log, TEXT("UpdateCachedDestination: Updated TrackedTarget to %s"), *HitRole->GetName());
        }
    }
}

// 新增：寻找范围内最近的敌人
AEnemyBase* AlivePlayerController::FindNearestEnemyInRange(float SearchRadius, const FVector& SearchOrigin)
{
	// 如果没有指定搜索原点，使用控制的角色位置
	FVector ActualSearchOrigin = SearchOrigin;
	if (ActualSearchOrigin.IsZero())
	{
		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			ActualSearchOrigin = ControlledPawn->GetActorLocation();
		}
		else
		{
			return nullptr;
		}
	}

	// 获取世界
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 使用球形碰撞检测范围内的所有 Actor
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(SearchRadius);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetPawn());

	// 检测范围内的所有 Pawn
	if (World->OverlapMultiByChannel(OverlapResults, ActualSearchOrigin, FQuat::Identity, ECC_Pawn, CollisionShape, CollisionParams))
	{
		AEnemyBase* NearestEnemy = nullptr;
		float NearestDistance = FLT_MAX;

		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* OverlappedActor = OverlapResult.GetActor();
			if (!OverlappedActor)
			{
				continue;
			}

			// 检查是否是敌人
			AEnemyBase* Enemy = Cast<AEnemyBase>(OverlappedActor);
			if (!Enemy)
			{
				continue;
			}

			// 检查敌人是否存活（HP > 0）
			if (Enemy->GetHealth() <= 0.0f)
			{
				continue;
			}

			// 计算距离
			float Distance = FVector::Dist(ActualSearchOrigin, Enemy->GetActorLocation());

			// 更新最近的敌人
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestEnemy = Enemy;
			}
		}

		return NearestEnemy;
	}

	return nullptr;
}

// 新增：开始追踪指定的 RoleBase
void AlivePlayerController::StartTracking(ARoleBase* Target, float SuccessDistance)
{
	// 如果目标无效，直接返回
	if (!Target)
	{
		UE_LOG(Loglive, Warning, TEXT("StartTracking: Target is null"));
		return;
	}

	// 停止当前的追踪
	StopTracking();

	// 设置新的追踪目标
	TrackedTarget = Target;
	TrackingSuccessDistance = SuccessDistance;
	bIsTracking = true;

	// 启动追踪定时器
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			TrackingTimerHandle,
			this,
			&AlivePlayerController::UpdateTracking,
			TrackingUpdateInterval,
			true
		);

		UE_LOG(Loglive, Log, TEXT("StartTracking: Started tracking %s"), *Target->GetName());
	}
}

// 新增：取消追踪
void AlivePlayerController::StopTracking()
{
	if (!bIsTracking)
	{
		return;
	}

	// 清除定时器
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TrackingTimerHandle);
	}

	// 停止移动
	StopMovement();

	// 清除追踪状态
	TrackedTarget = nullptr;
	bIsTracking = false;

	UE_LOG(Loglive, Log, TEXT("StopTracking: Stopped tracking"));
}

// 新增：追踪更新回调
void AlivePlayerController::UpdateTracking()
{
	// 检查目标是否仍然有效
	if (!TrackedTarget || !IsValid(TrackedTarget))
	{
		StopTracking();
		return;
	}

	// 检查目标是否存活
	if (TrackedTarget->GetHealth() <= 0.0f)
	{
		StopTracking();
		return;
	}

	// 获取控制的角色
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		StopTracking();
		return;
	}

	// 计算到目标的距离
	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TrackedTarget->GetActorLocation());

	// 如果已经到达目标距离，停止追踪
	if (Distance <= TrackingSuccessDistance)
	{
		StopMovement();
		UE_LOG(Loglive, Log, TEXT("UpdateTracking: Reached target at distance %f"), Distance);
		
		// 触发追踪成功委托
		if (OnTrackingSuccessDelegate.IsBound())
		{
			OnTrackingSuccessDelegate.Broadcast(TrackedTarget);
		}
		
		// 如果是自动攻击触发的追踪，触发普攻
		if (bIsAutoAttackTracking)
		{
			CastTrackingSuccessSkill();
			
			// 重置标志
			bIsAutoAttackTracking = false;
		}
		
		return;
	}

	// 移动向目标
	UAIBlueprintHelperLibrary::SimpleMoveToActor(this, TrackedTarget);
}

// 新增：设置角色行为状态
void AlivePlayerController::SetCharacterState(ECharacterBehaviorState NewState)
{
	// 如果状态没有变化，直接返回
	if (CurrentState == NewState)
	{
		return;
	}

	// 保存旧状态
	ECharacterBehaviorState OldState = CurrentState;

	// 根据旧状态进行清理
	switch (OldState)
	{
	case ECharacterBehaviorState::Tracking:
		// 从追踪状态切换出去，停止追踪
		if (NewState != ECharacterBehaviorState::Tracking)
		{
			StopTracking();
		}
		break;
	case ECharacterBehaviorState::Stunned:
		// 从眩晕状态切换出去，清除眩晕定时器
		if (NewState != ECharacterBehaviorState::Stunned)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				World->GetTimerManager().ClearTimer(StunTimerHandle);
			}
		}
		break;
	default:
		break;
	}

	// 设置新状态
	CurrentState = NewState;

	// 根据新状态进行初始化
	switch (NewState)
	{
	case ECharacterBehaviorState::Idle:
		// 站立状态，停止移动
		StopMovement();
		break;
	case ECharacterBehaviorState::Moving:
		// 移动状态，在 SetMovingState 中处理
		break;
	case ECharacterBehaviorState::Tracking:
		// 追踪状态，在 SetTrackingState 中处理
		break;
	case ECharacterBehaviorState::Stunned:
		// 眩晕状态，停止移动
		StopMovement();
		break;
	default:
		break;
	}

	// 触发状态切换委托
	if (OnStateChangedDelegate.IsBound())
	{
		OnStateChangedDelegate.Broadcast(NewState, OldState);
	}

	UE_LOG(Loglive, Log, TEXT("SetCharacterState: Changed from %d to %d"), (int32)OldState, (int32)NewState);
}

// 新增：切换到站立状态
void AlivePlayerController::SetIdleState()
{
	SetCharacterState(ECharacterBehaviorState::Idle);
}

// 新增：切换到移动状态
void AlivePlayerController::SetMovingState(const FVector& Destination)
{
	// 如果处于眩晕状态，不允许移动
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("SetMovingState: Cannot move while stunned"));
		return;
	}

	// 停止追踪
	if (CurrentState == ECharacterBehaviorState::Tracking)
	{
		StopTracking();
	}

	// 切换到移动状态
	SetCharacterState(ECharacterBehaviorState::Moving);

	// 移动到目标位置
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
}

// 新增：切换到追踪状态
void AlivePlayerController::SetTrackingState(ARoleBase* Target, float SuccessDistance)
{
	// 如果处于眩晕状态，不允许追踪
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("SetTrackingState: Cannot track while stunned"));
		return;
	}

	// 切换到追踪状态
	SetCharacterState(ECharacterBehaviorState::Tracking);

	// 开始追踪
	StartTracking(Target, SuccessDistance);
}

// 新增：切换到眩晕状态
void AlivePlayerController::SetStunnedState(float Duration)
{
	// 切换到眩晕状态
	SetCharacterState(ECharacterBehaviorState::Stunned);

	// 启动眩晕定时器
	UWorld* World = GetWorld();
	if (World && Duration > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			StunTimerHandle,
			[this]()
			{
				// 眩晕结束后切换到站立状态
				SetIdleState();
			},
			Duration,
			false
		);

		UE_LOG(Loglive, Log, TEXT("SetStunnedState: Stunned for %f seconds"), Duration);
	}
}

// 新增：Auto Attack 输入处理
void AlivePlayerController::OnAutoAttackTriggered()
{
	TrackingSuccessToCast = FName("Normal_Attack");
	StopTracking();
	StopTrackingSuccessSkill();
	// 如果处于眩晕状态，不允许操作
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("OnAutoAttackTriggered: Cannot act while stunned"));
		return;
	}

	// 更新鼠标位置
	UpdateCachedDestination();

	// 检查是否存在追踪目标
	if (TrackedTarget && IsValid(TrackedTarget))
	{
		// 如果目标存活，追击目标
		if (TrackedTarget->GetHealth() > 0.0f)
		{
			// 设置自动攻击追踪标志
			bIsAutoAttackTracking = true;
			SetTrackingState(TrackedTarget, TrackingSuccessDistance);
			UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered: Tracking target %s for auto attack"), *TrackedTarget->GetName());
		}
		else
		{
			// 目标已死亡，移动到鼠标位置
			SetMovingState(CachedDestination);
			UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered: Target is dead, moving to cursor position"));
		}
	}
	else
	{
		// 没有追踪目标，移动到鼠标位置
		SetMovingState(CachedDestination);
		UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered: No tracked target, moving to cursor position"));
	}
}

// 新增：触发追踪成功技能
void AlivePlayerController::CastTrackingSuccessSkill()
{
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		ARoleBase* RoleBase = Cast<ARoleBase>(ControlledPawn);
		if (RoleBase && RoleBase->GetSkillComponent() && TrackedTarget && IsValid(TrackedTarget))
		{
			// 释放追踪成功技能
			bool bCastSuccess = RoleBase->GetSkillComponent()->CastSkillByName(TrackingSuccessToCast, TrackedTarget, TrackedTarget->GetActorLocation());
			if (bCastSuccess)
			{
				UE_LOG(Loglive, Log, TEXT("CastTrackingSuccessSkill: Successfully cast skill %s on %s"), *TrackingSuccessToCast.ToString(), *TrackedTarget->GetName());
			}
			else
			{
				UE_LOG(Loglive, Warning, TEXT("CastTrackingSuccessSkill: Failed to cast skill %s"), *TrackingSuccessToCast.ToString());
			}
		}
	}
}

// 新增：停止追踪成功技能
void AlivePlayerController::StopTrackingSuccessSkill()
{
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		ARoleBase* RoleBase = Cast<ARoleBase>(ControlledPawn);
		if (RoleBase && RoleBase->GetAbilitySystemComponent())
		{
			// 只取消 TrackingSuccessToCast 技能
			FSkillData SkillData = USkillManager::Get()->GetSkillData(TrackingSuccessToCast);
			if (SkillData.SkillID > 0 && SkillData.AbilityClass.IsValid())
			{
				// 查找技能的 AbilitySpec
				FGameplayAbilitySpec* AbilitySpec = RoleBase->GetAbilitySystemComponent()->FindAbilitySpecFromClass(SkillData.AbilityClass.LoadSynchronous());
				if (AbilitySpec && AbilitySpec->Handle.IsValid())
				{
					// 取消特定技能
					RoleBase->GetAbilitySystemComponent()->CancelAbilityHandle(AbilitySpec->Handle);
				}
			}
		}
	}
}