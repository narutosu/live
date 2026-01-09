// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Role/RoleBase.h"
#include "Talent/TalentComponent.h"
#include "HeroBase.generated.h"

UCLASS()
class LIVE_API AHeroBase : public ARoleBase
{
	GENERATED_BODY()

public:
	AHeroBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	UTalentComponent* GetTalentComponent() const { return TalentComponent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
	class UTalentComponent* TalentComponent;
};
