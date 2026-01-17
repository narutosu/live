// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void UGameHelper::ShowToast(UObject* WorldContextObject, const FString& Message, float Duration)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// 获取玩家控制器
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (!PlayerController)
		{
			return;
		}

		// 加载 Widget 蓝图类
		TSoftClassPtr<UUserWidget> WidgetClassPtr = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UMG/WBP_Toast.WBP_Toast_C")));
		if (!WidgetClassPtr.IsValid())
		{
			WidgetClassPtr.LoadSynchronous();
		}

		UClass* WidgetClass = WidgetClassPtr.Get();
		if (!WidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load WBP_Main widget class"));
			return;
		}

		// 创建 Widget 实例
		UUserWidget* ToastWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
		if (!ToastWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create toast widget"));
			return;
		}

		// 查找 Text 控件并设置文本
		UTextBlock* TextBlock = Cast<UTextBlock>(ToastWidget->GetWidgetFromName(FName("Text")));
		if (TextBlock)
		{
			TextBlock->SetText(FText::FromString(Message));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TextBlock not found in WBP_Main widget"));
		}

		// 添加到视口
		ToastWidget->AddToViewport(0);

		// 使用定时器延迟销毁 Widget
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([ToastWidget]()
		{
			if (ToastWidget && ToastWidget->IsInViewport())
			{
				ToastWidget->RemoveFromParent();
			}
		});
		
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
	}
}
