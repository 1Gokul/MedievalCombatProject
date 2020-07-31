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
	InventoryComponent->InventoryName = FString("Container");

	InventoryComponent->NumberOfSlots = 8;
}

// Called when the game starts or when spawned
void AContainer::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AContainer::OnOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AContainer::OnOverlapEnd);

	// Add all elements of Items into the InventoryComponent
	for (int i = 0; i < Items.Num(); i++)
	{
		if (InventoryComponent->Inventory.IsValidIndex(i))
		{
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

void AContainer::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the OtherActor is the Character
	AMain* Main = Cast<AMain>(OtherActor);

	if(Main)
	{
		// If it is, display the Search Prompt.
		if(Main->MainPlayerController)
		{
			FString ItemName = InventoryComponent->InventoryName;
			FName SearchText("Search");
			
			Main->MainPlayerController->DisplaySearchPrompt(ItemName, SearchText);
		}
	}
}

void AContainer::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{  	
	// Check if the OtherActor is the Character
	AMain* Main = Cast<AMain>(OtherActor);

	if(Main)
	{
		// If it is, display the Search Prompt.
		if(Main->MainPlayerController)
		{			
			Main->MainPlayerController->RemoveSearchPrompt();
		}
	}
}