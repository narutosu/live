// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/ItemData.h"
#include "ItemActor.generated.h"

UCLASS()
class LIVE_API AItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Item name for this item actor (will load full data from ItemManager) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ExposeToSpawn = "true"))
	FName ItemName;

	/** Mesh component for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class UStaticMeshComponent* ItemMesh;

	/** Collision component for interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class USphereComponent* CollisionSphere;

	/** Niagara component for show effect (permanent effect) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class UNiagaraComponent* ShowEffectComponent;

	/** Whether this item can be picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBePickedUp;

	/** Whether this item has been picked up */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	bool bHasBeenPickedUp;

	/** Delegate for when item is picked up */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUp, AItemActor*, PickedUpItem);
	UPROPERTY(BlueprintAssignable, Category = "Item")
	FOnItemPickedUp OnItemPickedUp;

	/** Pick up this item */
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool PickUp(class ARoleBase* Picker);

	/** Get item data */
	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemData GetItemData() const { return ItemData; }

	/** Get item name */
	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetItemName() const { return ItemName; }

	/** Set item name */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemName(const FName& NewItemName) { ItemName = NewItemName; }

	/** Setup item appearance (mesh and effects) */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetupItemAppearance();

	/** Play drop animation with parabolic trajectory
	 * @param SpawnLocation The spawn location of the item
	 * @param DropRadius The radius within which the item will land (default 300)
	 * @param DropHeight The maximum height of the parabolic arc (default 200)
	 * @param DropDuration The duration of the drop animation in seconds (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void PlayDropAnimation(const FVector& SpawnLocation, float DropRadius = 300.0f, float DropHeight = 200.0f, float DropDuration = 1.0f);

protected:
	/** Called when item is successfully picked up */
	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void OnPickedUp(class ARoleBase* Picker);
	virtual void OnPickedUp_Implementation(class ARoleBase* Picker);

	/** Handle overlap with other actors */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** Cached item data loaded from ItemManager */
	UPROPERTY()
	FItemData ItemData;

	/** Whether drop animation is currently playing */
	UPROPERTY()
	bool bIsPlayingDropAnimation;

	/** Current drop animation time */
	UPROPERTY()
	float DropAnimationTime;

	/** Total drop animation duration */
	UPROPERTY()
	float DropAnimationDuration;

	/** Start location for drop animation */
	UPROPERTY()
	FVector DropStartLocation;

	/** Target location for drop animation */
	UPROPERTY()
	FVector DropTargetLocation;

	/** Maximum height for parabolic arc */
	UPROPERTY()
	float DropMaxHeight;
};
