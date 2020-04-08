// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Critter.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASpawnVolume::GetSpawnPoint()
{
	FVector Extent = SpawningBox->GetScaledBoxExtent();		//Size of Spawning box
	FVector Origin = SpawningBox->GetComponentLocation();	//Position of Spawning Box

	return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
}

void ASpawnVolume::SpawnOurPawn_Implementation(UClass* ToSpawn, const FVector& Location)
{
	
	if (ToSpawn) {	

		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;

		if (World) {
			ACritter* SpawnedCritter = World->SpawnActor<ACritter>(ToSpawn, Location, FRotator(0.0f), SpawnParams);
		}
	}
}

