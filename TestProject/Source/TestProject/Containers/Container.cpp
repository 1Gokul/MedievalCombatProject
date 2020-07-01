// Fill out your copyright notice in the Description page of Project Settings.


#include "Container.h"
#include <Components/BoxComponent.h>

#include "Main.h"
#include "MainPlayerController.h"

// Sets default values
AContainer::AContainer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Container Collision Box
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// Container Mesh
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
	
	// Setup Inventory
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	InventoryComponent->InventoryName = FText::FromString("Container");
	
	InventoryComponent->NumberOfSlots = 8;

}

// Called when the game starts or when spawned
void AContainer::BeginPlay()
{
	Super::BeginPlay();

	// Add all elements of Items into the InventoryComponent
	for (int i = 0; i<Items.Num();i++)
	{
		if(InventoryComponent->Inventory.IsValidIndex(i)){
			InventoryComponent->Inventory[i] = Items[i];
		}
	}

	
}

// Called every frame
void AContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("Container NumberOfSlots = %i"), InventoryComponent->NumberOfSlots);

}

void AContainer::Interact(AActor* Interacter)
{
	// Cast to Main
	AMain* Main = Cast<AMain>(Interacter);

	// Display this Container's Inventory
	Main->MainPlayerController->ToggleInventoryMenu(InventoryComponent);
}

