// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"


USTRUCT(BlueprintType)
struct FItemStructure
{
	GENERATED_BODY()

	//Constructor
	FItemStructure();

	/** The display name for this item in the inventory */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	FName ItemDisplayName;

	/** The Text for using the Item i.e "Eat", "Equip" etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	FName UseActionText;

	/** Optional description of the item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info", meta = (MultiLine = true))
	FString ItemDescription;

	/** The weight of the item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info", meta = (ClampMin = 0.0))
	float Weight;

	/** If true, multiple Items of this type can be stacked together. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	bool bIsStackable;

	/** The thumbnail for this item in the menu*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	class UTexture2D* Thumbnail;

	/** Maximum number of Items of this type that can be stacked together. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	int32 MaxStackSize;

	/** If the Item is consumable or not*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	bool bIsConsumable;

	/** If the Item is equippable or not*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	bool bIsEquippable;

	/** If the Item is equippable or not*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	int32 Value;

	/** Item class reference */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Info")
	TSubclassOf<class AItem> ItemClass;
};


UCLASS(Abstract, BlueprintType, Blueprintable)
class TESTPROJECT_API AItem : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

	UPROPERTY(Transient)
	class UWorld* World;

	/** Item Stats*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemStructure")
	FItemStructure ItemStructure;

	/** Base shape collision*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision")
	class USphereComponent* CollisionVolume;

	/**Item mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh")
	class UStaticMeshComponent* StaticMesh;

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

	///** The Inventory that owns this item*/
	//UPROPERTY()
	//class UInventoryComponent* OwningInventory; 


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

	/** Override Interact() function from IInteractInterface class */
	void Interact(AActor* Interacter) override;

	//virtual void Use(class AMain* Main) PURE_VIRTUAL(AItem, );

	/**
	 * This Pure Virtual function gets called when the Player clicks on an Item in the Inventory Menu.
	 * Different implementations of this function will be done in the Child classes.
	 *
	 * Is called in the InventoryItem_BP Widget.
	 *
	 * @returns true if the Item was used successfully.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool UseItem(class AMain* Main);

	//virtual class UWorld* ItemGetWorld() const {return World;};
};
