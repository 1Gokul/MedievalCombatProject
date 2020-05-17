// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Shield.generated.h"


UENUM(BlueprintType)
enum class EShieldState : uint8 {
	ESS_Pickup		UMETA(DisplayName = "Pickup"),
	ESS_Equipped	UMETA(DisplayName = "Equipped"),

	ESS_MAX			UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class TESTPROJECT_API AShield : public AItem
{
	GENERATED_BODY()


public:

	AShield();

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	EShieldState ShieldState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	USoundCue* BlockSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items | Block")
	class UBoxComponent* BlockCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
		float BlockStaminaCost;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items | Block")
		AController* ShieldInstigator;

protected:

	virtual void BeginPlay() override;

public:

	/**Called if an Overlap Event starts. */
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	/** Called when the Overlap Event ends. */
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

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
