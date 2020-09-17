// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Weapon.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()

	FName HitSocketName;

	// The Attack number of the Character. Used to call the appropriate Hit Reaction animation for the Enemy if Hit. 
	int32 MainAttackSection;

public:

	AMeleeWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items | Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	TArray<USoundCue*> SwingSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	USoundCue* BlockSound;

	/** Particles emitted when the weapon gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
	float BlockStaminaCost;

protected:
	void BeginPlay() override;

public:

	/** Below two functions are similar to the above OnOverlapBegin and End,
	* but they are not overriden as they are for the Combat Collision Box 
	* which is not a part of the parent Item class. */

	/**Called if a Combat Overlap Event starts. */
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

	/** Called when the Combat Overlap Event ends. */
	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	void PlayBlockSound();

	void EmitHitParticles();
	FORCEINLINE void SetMainAttackSection(int32 AttackSection) { MainAttackSection = AttackSection; }
};
