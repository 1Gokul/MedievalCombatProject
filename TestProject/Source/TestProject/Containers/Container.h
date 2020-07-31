// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "Container.generated.h"

UCLASS()
class TESTPROJECT_API AContainer : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AContainer();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container")
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemsToAdd", meta = (ExposeOnSpawn = true))
	TArray<struct FSlotStructure> Items;


protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:
	// Called every frame
	void Tick(float DeltaTime) override;

	/** Override InteractInterface */
	void Interact(AActor* Interacter) override;

	/**Called if an Overlap Event starts. */
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                            const FHitResult& SweepResult);

	/** Called when the Overlap Event ends */
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};