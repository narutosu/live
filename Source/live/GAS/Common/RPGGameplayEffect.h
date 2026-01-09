// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "RPGGameplayEffect.generated.h"

/**
 * 
 */
UCLASS()
class LIVE_API URPGGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	// 一些技能的说明，用于状态栏显示
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FText ItemDescription;

	// 有技能图标则显示，否则不显示
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FSlateBrush ItemIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Max)
	int32 Count;
};
