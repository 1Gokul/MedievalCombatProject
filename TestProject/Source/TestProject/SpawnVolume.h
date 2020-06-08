// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class TESTPROJECT_API ASpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Spawning)
	class UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<AActor> Actor_1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<AActor> Actor_2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<AActor> Actor_3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<AActor> Actor_4;

	TArray<TSubclassOf<AActor>> SpawnArray;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:
	// Called every frame
	void Tick(float DeltaTime) override;

	//Get a random point in the Spawn Volume to spawn the Actor
	UFUNCTION(BlueprintPure, Category = Spawning)
	FVector GetSpawnPoint();

	//Get a random Actor to spawn
	UFUNCTION(BlueprintPure, Category = Spawning)
	TSubclassOf<AActor> GetSpawnActor();

	//Spawn the Actor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Spawning)
	void SpawnOurActor(UClass* ToSpawn, const FVector& Location);
};
