// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class TESTPROJECT_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

	UPROPERTY(Transient)
	class UWorld* World;
	

	/** Base shape collision*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision")
	class USphereComponent* CollisionVolume;

	/**Item mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh")
	class UStaticMeshComponent* StaticMesh;

	/** The Text for using the Item i.e "Eat", "Equip" etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	FName UseActionText;

	/** The thumbnail for this item in the menu*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item | Info")
	class UTexture2D* Thumbnail;

	/** The display name for this item in the inventory */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	FName ItemDisplayName;

	/** Optional description of the item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info", meta = (MultiLine = true))
	FString ItemDescription;

	/** The weight of the item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info", meta = (ClampMin = 0.0))
	float Weight;

	/** Particle Component when the item is idle and has not been picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystemComponent* IdleParticlesComponent;

	/** Particle System that is emitted when the player overlaps with the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystem* OverlapParticles;

	/** Sound emitted when overlap occurs*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sounds")
	class USoundCue* OverlapSound;

	/** Bool to check if the item should rotate when idle or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	bool bShouldRotate;

	/** Rate at which the item will rotate if bShouldRotate = true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	float RotationRate;

	/** The Inventory that owns this item */
	UPROPERTY()
	class UInventoryComponent* OwningInventory;

	
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:
	// Called every frame
	void Tick(float DeltaTime) override;

	/**Called if an Overlap Event starts. */
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                            const FHitResult& SweepResult);

	/** Called when the Overlap Event ends */
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Use(class AMain* Main) PURE_VIRTUAL(AItem, );

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(AMain* Main);

	virtual class UWorld* ItemGetWorld() const {return World;};
	
};


