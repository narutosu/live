// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Role/RoleBase.h"
#include "Role/HeroBase.h"
#include "Item/InventoryComponent.h"
#include "Item/ItemManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the mesh component
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetupAttachment(RootComponent);

	// Create the collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(50.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionSphere->SetupAttachment(ItemMesh);

	// Create the Niagara component for show effect (permanent effect)
	ShowEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShowEffectComponent"));
	ShowEffectComponent->SetupAttachment(ItemMesh);
	ShowEffectComponent->bAutoActivate = false;

	// Set default values
	bCanBePickedUp = true;
	bHasBeenPickedUp = false;
	
	// Initialize drop animation variables
	bIsPlayingDropAnimation = false;
	DropAnimationTime = 0.0f;
	DropAnimationDuration = 1.0f;
	DropMaxHeight = 200.0f;
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetupItemAppearance();
	// Bind overlap event
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnOverlapBegin);
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update drop animation
	if (bIsPlayingDropAnimation && !bHasBeenPickedUp)
	{
		DropAnimationTime += DeltaTime;
		if (DropAnimationTime >= DropAnimationDuration)
		{
			// Animation complete
			bIsPlayingDropAnimation = false;
			SetActorLocation(DropTargetLocation);
			
			// Find all HeroBase actors within 300 units
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHeroBase::StaticClass(), FoundActors);
			
			for (AActor* Actor : FoundActors)
			{
				AHeroBase* Hero = Cast<AHeroBase>(Actor);
				if (Hero && !Hero->IsPendingKillPending())
				{
					float Distance = FVector::Dist(GetActorLocation(), Hero->GetActorLocation());
					if (Distance <= 300.0f)
					{
						// Hero found in range, try to pick up
						PickUp(Hero);
						break; // Only pick up by one hero
					}
				}
			}
		}
		else
		{
			// Calculate parabolic trajectory
			float Alpha = DropAnimationTime / DropAnimationDuration;
			
			// Linear interpolation between start and target
			FVector CurrentLocation = FMath::Lerp(DropStartLocation, DropTargetLocation, Alpha);
			
			// Add parabolic height offset (sin curve for smooth arc)
			float HeightOffset = FMath::Sin(Alpha * PI) * DropMaxHeight;
			CurrentLocation.Z += HeightOffset;
			
			// Update actor position
			SetActorLocation(CurrentLocation);
		}
	}

	// Optional: Rotate the item for visual effect
	if (ItemMesh && !bHasBeenPickedUp)
	{
		FRotator NewRotation = ItemMesh->GetRelativeRotation();
		NewRotation.Yaw += DeltaTime * 30.0f; // Rotate 30 degrees per second
		ItemMesh->SetRelativeRotation(NewRotation);
	}
}

void AItemActor::SetupItemAppearance()
{
	// Load item data from ItemManager using ItemName
	if (ItemName == NAME_None)
	{
		return;
	}
	
	FItemData LoadedData = UItemManager::Get()->GetItemData(ItemName);
	if (LoadedData.ItemID <= 0)
	{
		return;
	}
	ItemData = LoadedData;
	// Load and set the item mesh
	if (ItemData.ItemMesh.IsValid())
	{
		UStaticMesh* LoadedMesh = ItemData.ItemMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			ItemMesh->SetStaticMesh(LoadedMesh);
		}
	}
	
	// Load and activate the show effect (permanent effect)
	if (ItemData.ShowEffect.IsValid())
	{
		UNiagaraSystem* LoadedEffect = ItemData.ShowEffect.LoadSynchronous();
		if (LoadedEffect)
		{
			ShowEffectComponent->SetAsset(LoadedEffect);
			ShowEffectComponent->Activate();
		}
	}
}

bool AItemActor::PickUp(ARoleBase* Picker)
{
	if (!Picker || !bCanBePickedUp || bHasBeenPickedUp)
	{
		return false;
	}

	// Check if picker has inventory component
	UInventoryComponent* Inventory = Picker->GetInventoryComponent();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemActor::PickUp: Picker has no inventory component"));
		return false;
	}

	// Try to add item to inventory
	bool bAdded = false;
	
	// Use ItemName to add item to inventory
	if (ItemName != NAME_None)
	{
		bAdded = Inventory->AddItem(ItemName, 1);
	}

	if (bAdded)
	{
		bHasBeenPickedUp = true;
		
		// Call blueprint event
		OnPickedUp(Picker);
		
		// Broadcast delegate
		OnItemPickedUp.Broadcast(this);
		
		// Hide the item
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		
		// Destroy after a short delay
		SetLifeSpan(0.1f);
		
		return true;
	}

	return false;
}

void AItemActor::OnPickedUp_Implementation(ARoleBase* Picker)
{
	// Default implementation - can be overridden in Blueprint
	UE_LOG(LogTemp, Log, TEXT("ItemActor::OnPickedUp: Item %s picked up by %s"), 
		*ItemName.ToString(), 
		Picker ? *Picker->GetName() : TEXT("Unknown"));
}

void AItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping actor is a role and can pick up items
	ARoleBase* RoleBase = Cast<ARoleBase>(OtherActor);
	if (RoleBase && bCanBePickedUp && !bHasBeenPickedUp)
	{
		// Only auto-pickup if the item's pickup mode is set to AutoPickup
		if (ItemData.PickupMode == EItemPickupMode::AutoPickup)
		{
			PickUp(RoleBase);
		}
	}
}

void AItemActor::PlayDropAnimation(const FVector& SpawnLocation, float DropRadius, float DropHeight, float DropDuration)
{
	// Set animation parameters
	DropStartLocation = SpawnLocation;
	DropMaxHeight = DropHeight;
	DropAnimationDuration = DropDuration;
	DropAnimationTime = 0.0f;
	
	// Calculate random target location within the specified radius
	float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
	float RandomDistance = FMath::FRandRange(0.0f, DropRadius);
	
	DropTargetLocation.X = SpawnLocation.X + FMath::Cos(RandomAngle) * RandomDistance;
	DropTargetLocation.Y = SpawnLocation.Y + FMath::Sin(RandomAngle) * RandomDistance;
	DropTargetLocation.Z = SpawnLocation.Z; // Keep same Z level for landing
	
	// Start the animation
	bIsPlayingDropAnimation = true;
	
	// Set initial position
	SetActorLocation(SpawnLocation);
}
