// Fill out your copyright notice in the Description page of Project Settings.

#include "Role/HeroBase.h"

AHeroBase::AHeroBase()
{
	// Create TalentComponent for heroes
	TalentComponent = CreateDefaultSubobject<UTalentComponent>(TEXT("TalentComponent"));
	//
}

void AHeroBase::BeginPlay()
{
	Super::BeginPlay();
	SkillComponent->LearnSkillByName(FName("Normal_Attack"));
	SkillComponent->LearnSkillByName(FName("Critical_Hit"));
	SkillComponent->LearnSkillByName(FName("Sacrifice"));
	SkillComponent->LearnSkillByName(FName("Phase_Transfer"));
	SkillComponent->LearnSkillByName(FName("ShengXinTaiBao"));
}

void AHeroBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHeroBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
