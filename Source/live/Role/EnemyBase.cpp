// Fill out your copyright notice in the Description page of Project Settings.

#include "Role/EnemyBase.h"

AEnemyBase::AEnemyBase()
{
	// EnemyBase inherits from RoleBase but does not use TalentComponent
	// TalentComponent will be nullptr for enemies
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
