// Fill out your copyright notice in the Description page of Project Settings.


#include "Floater.h"

// Sets default values
AFloater::AFloater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomStaticMesh"));

	InitialLocation = FVector(0.0f, 0.0f, 0.0f);
	PlacedLocation = FVector(0.0f, 0.0f, 0.0f);
	WorldOrigin = FVector(0.0f, 0.0f, 0.0f);
	InitialDirection = FVector(0.0f, 0.0f, 0.0f);

	bInitializeFloaterLocation = false;
	bShouldFloat = false;

	RunningTime = 0.0f;

	Amplitude = 1.0f;
	TimePeriod = 1.0f;
	StartingPosition = 1.0f;
}

// Called when the game starts or when spawned
void AFloater::BeginPlay()
{
	Super::BeginPlay();
	
	PlacedLocation = GetActorLocation();

	if (bInitializeFloaterLocation) {
		SetActorLocation(InitialLocation);
	}
	
}

// Called every frame
void AFloater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldFloat) {

		FVector NewLocation = GetActorLocation();

		NewLocation.Z = NewLocation.Z + Amplitude * FMath::Sin(TimePeriod * RunningTime) + StartingPosition;
		//NewLocation.Y = NewLocation.Z + Amplitude * FMath::Cos(TimePeriod * RunningTime) + StartingPosition;

		SetActorLocation(NewLocation);

		RunningTime += DeltaTime;
	}

}

