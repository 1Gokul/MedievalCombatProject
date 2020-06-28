// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"


#include "Shield.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "InventoryComponent.h"


FItemStructure::FItemStructure(): Thumbnail(nullptr)
{
	ItemDisplayName = FName("None");

	UseActionText = FName("Use");

	bIsStackable = true;

	ItemDescription = FString("None");

	MaxStackSize = 4;

	bIsConsumable = true;
	bIsEquippable = false;

	Weight = 1.0f;
}


// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Volume"));
	RootComponent = CollisionVolume;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(GetRootComponent());

	IdleParticlesComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IdleParticlesComponent"));
	IdleParticlesComponent->SetupAttachment(GetRootComponent());

	bShouldRotate = false;
	RotationRate = 45.0f;
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
	CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldRotate)
	{
		FRotator CurrentRotation = GetActorRotation();
		CurrentRotation.Yaw += DeltaTime * RotationRate;

		SetActorRotation(CurrentRotation);
	}
}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex)
{
}

void AItem::Interact(AActor* Interacter)
{
	UE_LOG(LogTemp, Warning, TEXT("InteractInterface::Interact() has been called!"));

	// Get the InventoryComponent of the Interacter and add the Items to its Inventory.

	UActorComponent* InventoryComponent = Interacter->GetComponentByClass(UInventoryComponent::StaticClass());

	UInventoryComponent* OwningInventory = Cast<UInventoryComponent>(InventoryComponent);

	if(OwningInventory)
	{
		//Add values to a SlotStructure Object
		FSlotStructure SlotStructure = FSlotStructure();
		SlotStructure.ItemStructure = ItemStructure;
		SlotStructure.Quantity = 1;

		// Add the object to the Inventory
		OwningInventory->AddToInventory(SlotStructure);

		Destroy();
	}
}
