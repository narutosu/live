// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "GameHelper.generated.h"

UCLASS()
class LIVE_API UGameHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 显示 Toast 消息
	 * @param WorldContextObject 世界上下文对象
	 * @param Message 要显示的消息文本
	 * @param Duration 显示持续时间（秒），默认 3 秒
	 */
	UFUNCTION(BlueprintCallable, Category = "GameHelper|UI", meta = (WorldContext = "WorldContextObject"))
	static void ShowToast(UObject* WorldContextObject, const FString& Message, float Duration = 3.0f);
};
