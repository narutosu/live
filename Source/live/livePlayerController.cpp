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
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Skill/SkillManager.h"
#include "Skill/SkillData.h"
#include "GAS/Common/RPGGameplayAbility.h"
#include "Item/ItemActor.h"
#include "Kismet/KismetSystemLibrary.h"

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
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AlivePlayerController::OnSetDestinationTriggered);
			// Setup right click input event
			EnhancedInputComponent->BindAction(SetDestinationRightClickAction, ETriggerEvent::Started, this, &AlivePlayerController::OnSetDestinationRightClickTriggered);
			// Setup auto attack input event
			EnhancedInputComponent->BindAction(AutoAttackAction, ETriggerEvent::Started, this, &AlivePlayerController::OnAutoAttackTriggered);
			
		}
		else
		{
			UE_LOG(Loglive, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		}
	}
}

void AlivePlayerController::OnSetDestinationTriggered()
{
	UE_LOG(Loglive, Log, TEXT("========================================OnSetDestinationTriggered"));
}

// 新增：Auto Attack 输入处理
void AlivePlayerController::OnAutoAttackTriggered()
{
	StopTracking();
	StopTrackingSuccessSkill();
	// 如果处于眩晕状态，不允许操作
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("OnAutoAttackTriggered: Cannot act while stunned"));
		return;
	}
	
	// 配置碰撞查询参数，忽略当前控制的 Pawn
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetPawn());

	// 获取鼠标位置和方向
	FVector MouseLocation, MouseDirection;
	DeprojectMousePositionToWorld(MouseLocation, MouseDirection);

	// 设置射线起点和终点
	FVector TraceStart = MouseLocation;
	FVector TraceEnd = MouseLocation + MouseDirection * 10000.0f;

	// 检测射线是否击中 ItemActor
	FHitResult Hit;
	bool bHitSuccessful = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel4, CollisionParams);
	AActor* HitActor = Hit.GetActor();
	if (bHitSuccessful && HitActor)
	{
		ARoleBase* RoleActor = Cast<ARoleBase>(HitActor);
		// 检查是否点击到了 RoleBase（敌人或友方角色）
		if (RoleActor && IsValid(RoleActor))
		{
			// 更新追踪目标
			TrackedTarget = RoleActor;
			UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered:  TrackedTarget to %s"), *RoleActor->GetName());
		}
		else
        {
			AEnemyBase* EnemyActor = FindNearestEnemyInRange(350, Hit.Location);
			if (EnemyActor && IsValid(EnemyActor))
            {
                TrackedTarget = EnemyActor;
				UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered:Auto Find Nearest Enemy TrackedTarget to %s"), *EnemyActor->GetName());
            }
        }
	}
	
	// 检查是否存在追踪目标
	if (TrackedTarget && IsValid(TrackedTarget))
	{
		SetTrackingState(TrackedTarget,100, FName("Normal_Attack"));
	}
	else
	{
		// 没有追踪目标，移动到鼠标位置
		SetMovingState(Hit.Location);
		UE_LOG(Loglive, Log, TEXT("OnAutoAttackTriggered: No tracked target, moving to cursor position"));
	}
}


// 新增：右键点击输入处理
void AlivePlayerController::OnSetDestinationRightClickTriggered()
{
	UE_LOG(Loglive, Log, TEXT("========================================OnSetDestinationRightClickTriggered"));
	// 如果处于眩晕状态，不允许操作
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("OnSetDestinationRightClickTriggered: Cannot act while stunned"));
		return;
	}

	// 停止当前的追踪和技能
	StopTracking();
	StopTrackingSuccessSkill();

	// 配置碰撞查询参数，忽略当前控制的 Pawn
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetPawn());

	// 获取鼠标位置和方向
	FVector MouseLocation, MouseDirection;
	DeprojectMousePositionToWorld(MouseLocation, MouseDirection);

	// 设置射线起点和终点
	FVector TraceStart = MouseLocation;
	FVector TraceEnd = MouseLocation + MouseDirection * 10000.0f;

	// 检测射线是否击中 ItemActor
	FHitResult Hit;
	bool bHitSuccessful = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel3, CollisionParams);
	AActor* HitActor = Hit.GetActor();
	if (bHitSuccessful && HitActor)
	{
		// 检查是否点击到了 ItemActor
		AItemActor* ItemActor = Cast<AItemActor>(HitActor);
		ARoleBase* RoleActor = Cast<ARoleBase>(HitActor);
		CachedDestination = Hit.Location;
		// 检查是否点击到了 RoleBase（敌人或友方角色）
		if (RoleActor && IsValid(RoleActor))
		{
			SetTrackingState(RoleActor, TrackingSuccessDistance, FName("Normal_Attack"));
			UE_LOG(Loglive, Log, TEXT("UpdateCachedDestination: Updated TrackedTarget to %s"), *RoleActor->GetName());
			return;
		}
		else if (ItemActor && IsValid(ItemActor))
		{
			// 点击的是 ItemActor，移动到附近并拾取
			// 存储要拾取的物品
			ItemToPickUp = ItemActor;
			// 移动到 ItemActor 附近
			SetMovingState(Hit.Location);
			
			UE_LOG(Loglive, Log, TEXT("OnSetDestinationRightClickTriggered: Moving to pick up item %s"), *ItemActor->GetName());
			return;
		}
		else
		{
			
			SetMovingState(Hit.Location);
			// 生成点击特效
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(0.5f, 0.5f, 0.5f), true, true, ENCPoolMethod::None, true);
	
		}
	}

	UE_LOG(Loglive, Log, TEXT("OnSetDestinationRightClickTriggered: Moving to destination %s"), *CachedDestination.ToString());
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

	// 绑定目标的死亡委托
	if (TrackedTarget)
	{
		TrackedTarget->OnDeathDelegate.AddDynamic(this, &AlivePlayerController::OnTrackedTargetDeath);
	}

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

	// 解绑目标的死亡委托
	if (TrackedTarget)
	{
		TrackedTarget->OnDeathDelegate.Remove(this, GET_FUNCTION_NAME_CHECKED(AlivePlayerController, OnTrackedTargetDeath));
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
		
		// 如果是自动攻击触发的追踪
		if (!TrackingSuccessToCast.IsNone())
		{
			CastTrackingSuccessSkill();
		}
		
		return;
	}

	// 移动向目标
	UAIBlueprintHelperLibrary::SimpleMoveToActor(this, TrackedTarget);
}

// 新增：设置角色行为状态
void AlivePlayerController::SetCharacterState(ECharacterBehaviorState NewState)
{
	// 保存旧状态
	ECharacterBehaviorState OldState = CurrentState;
	
	//清理旧状态
	StopTracking();
	StopTrackingSuccessSkill();

	// 设置新状态
	CurrentState = NewState;
	OnStateChangedDelegate.Broadcast(NewState, OldState);

	// 根据新状态进行初始化
	switch (NewState)
	{
	case ECharacterBehaviorState::Idle:
		// 站立状态，停止移动
		StopMovement();
		// 清除拾取物品定时器
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TrackingTimerHandle);
		}
		break;
	case ECharacterBehaviorState::Moving:
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
}

// 新增：切换到站立状态
void AlivePlayerController::SetIdleState()
{
	SetCharacterState(ECharacterBehaviorState::Idle);
}

// 新增：切换到移动状态
void AlivePlayerController::SetMovingState(const FVector& Destination)
{
	CachedDestination = Destination;
	// 如果处于眩晕状态，不允许移动
	if (CurrentState == ECharacterBehaviorState::Stunned)
	{
		UE_LOG(Loglive, Warning, TEXT("SetMovingState: Cannot move while stunned"));
		return;
	}

	// 切换到移动状态
	SetCharacterState(ECharacterBehaviorState::Moving);
	
	if (ItemToPickUp && IsValid(ItemToPickUp))
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(TrackingTimerHandle);
			World->GetTimerManager().SetTimer(
				TrackingTimerHandle,
				this,
				&AlivePlayerController::UpdateItemPickup,
				TrackingUpdateInterval,
				true
			);
		}
	}

	// 移动到目标位置
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
}

// 新增：拾取物品更新回调
void AlivePlayerController::UpdateItemPickup()
{
	// 检查物品是否仍然有效
	if (!ItemToPickUp || !IsValid(ItemToPickUp))
	{
		// 物品已无效，清除定时器和引用
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TrackingTimerHandle);
		}
		ItemToPickUp = nullptr;
		return;
	}

	// 获取控制的角色
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// 计算到物品的距离
	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), ItemToPickUp->GetActorLocation());

	// 如果已经到达拾取距离，尝试拾取
	if (Distance <= 300.0f)
	{
		// 停止移动
		StopMovement();
		
		// 尝试拾取物品
		ARoleBase* RoleBase = Cast<ARoleBase>(ControlledPawn);
		if (RoleBase)
		{
			bool bPickedUp = ItemToPickUp->PickUp(RoleBase);
			if (bPickedUp)
			{
				UE_LOG(Loglive, Log, TEXT("UpdateItemPickup: Successfully picked up item %s"), *ItemToPickUp->GetName());
			}
			else
			{
				UE_LOG(Loglive, Warning, TEXT("UpdateItemPickup: Failed to pick up item %s"), *ItemToPickUp->GetName());
			}
		}
		
		// 清除定时器和引用
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TrackingTimerHandle);
		}
		ItemToPickUp = nullptr;
		
		// 切换到站立状态
		SetIdleState();
	}
}

// 新增：切换到追踪状态
void AlivePlayerController::SetTrackingState(ARoleBase* Target, float SuccessDistance,FName ParamTrackingSuccessToCast)
{
	// 如果目标无效，直接返回
	if (!Target)
	{
		UE_LOG(Loglive, Warning, TEXT("TrackedAndAttackTarget: Target is null"));
		return;
	}

	// 设置追踪成功时要释放的技能
	this->TrackingSuccessToCast = ParamTrackingSuccessToCast;
	
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
			// 平滑旋转朝向追踪目标
			RoleBase->SmoothRotateToLocation(TrackedTarget->GetActorLocation());
			
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

// 新增：追踪目标死亡时的回调
void AlivePlayerController::OnTrackedTargetDeath()
{
	UE_LOG(Loglive, Log, TEXT("OnTrackedTargetDeath: Tracked target died, stopping tracking"));
	
	// 停止追踪
	StopTracking();
	StopTrackingSuccessSkill();
}