// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Main.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "InventoryComponent.h"
#include "MainPlayerController.h"


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
	Value = 1;
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
	// Check if the OtherActor is the Character
	AMain* Main = Cast<AMain>(OtherActor);

	if(Main)
	{
		// If it is, display the Item Interact Prompt.
		if(Main->MainPlayerController)
		{
			// ItemInteract text has been defined as "Take". 
			Main->MainPlayerController->DisplayItemInteractPrompt(ItemStructure.ItemDisplayName.ToString(), ItemStructure.Weight, ItemStructure.Value, ItemInteractText);
		}
	}
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex)
{
	// Check if the OtherActor is the Character
	AMain* Main = Cast<AMain>(OtherActor);

	if(Main)
	{
		// If it is, display the Item Interact Prompt.
		if(Main->MainPlayerController)
		{			
			Main->MainPlayerController->RemoveItemInteractPrompt();
		}
	}
}

void AItem::Interact(AActor* Interacter)
{
	UE_LOG(LogTemp, Warning, TEXT("InteractInterface::Interact() has been called!"));

	/*// Get the InventoryComponent of the Interacter and add the Items to its Inventory.
	 *UActorComponent* InventoryComponent = Interacter->GetComponentByClass(UInventoryComponent::StaticClass());

	UInventoryComponent* OwningInventory = Cast<UInventoryComponent>(InventoryComponent);*/

	//// Disable Collision Volume of Item
	/*CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);*/

	// Cast Interacter to Main
	AMain* Main = Cast<AMain>(Interacter);

	if (Main)
	{
		Main->ResetIdleTimer();
		
		//Add values to a SlotStructure Object
		FSlotStructure SlotStructure;
		SlotStructure.ItemStructure = ItemStructure;
		SlotStructure.Quantity = 1;

		// Add the object to the Inventory
		bool Success = Main->InventoryComponent->AddToInventory(SlotStructure);

		if (Success)Destroy();
	}
}

bool AItem::UseItem(AMain* Main)
{
	Main->ResetIdleTimer();
	
	return true;
}
