// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainShield.generated.h"

UENUM(BlueprintType)
enum class EShieldState : uint8 {
	ESS_Pickup		UMETA(DisplayName = "Pickup"),
	ESS_Equipped	UMETA(DisplayName = "Equipped"),

	ESS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class TESTPROJECT_API AMainShield : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMainShield();

	// Root component of the Shield
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield | Collision")
	class UBoxComponent* BlockCollision;
	
	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shield | State")
	EShieldState ShieldState;

	/** Surrounding collision sphere. Used for picking up and equipping the shield.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shield | Collision")
	class USphereComponent* PickupVolume;

	/**Item mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shield | Mesh")
	class UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	class USoundCue* OnEquipSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield | Sounds")
	class USoundCue* BlockSound;	 

	/** Sound emitted when overlap occurs*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield | Sounds")
	class USoundCue* OverlapSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
		float BlockStaminaCost;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items | Block")
		AController* ShieldInstigator;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

		/**Called if an Overlap Event starts. */
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the Overlap Event ends */
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Below two functions are similar to the above OnOverlapBegin and End,
	*	but they are not overriden as they are for the Combat Collision Box
	*	which is not a part of the parent Item class. */

	/**Called if a Combat Overlap Event starts. */
	UFUNCTION()
		void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the Combat Overlap Event ends. */
	UFUNCTION()
		void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void ActivateCollision();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollision();

	void Equip(class AMain* Char);

	FORCEINLINE void SetShieldState(EShieldState State) { ShieldState = State; }
	FORCEINLINE EShieldState GetShieldState() { return ShieldState; }
	FORCEINLINE void SetInstigator(AController* Instigator) { ShieldInstigator = Instigator; }
};


