// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	EWS_Pickup		UMETA(DisplayName = "Pickup"),
	EWS_Equipped	UMETA(DisplayName = "Equipped"),

	EWS_MAX			UMETA(DisplayName = "DefaultMAX")
};
  
/**
 * 
 */
UCLASS()
class TESTPROJECT_API AWeapon : public AItem
{
	GENERATED_BODY()

public:

	AWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	EWeaponState WeaponState; 

	/** To choose if the weapon's particle effects should show even after equipping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Particles")
		bool bWeaponParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
		class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	USoundCue* OnSheathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	TArray<USoundCue*>SwingSounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeletal Mesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items | Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
		TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		AController* WeaponInstigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Sound")
	USoundCue* BlockSound;

	/** Particles emitted when the weapon gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | Block")
		float BlockStaminaCost;

	FName HitSocketName;

	int32 MainAttackSection;

	/**If true, the Player will treat the weapon as two handed.
	 * No Shield can be equipped if the CurrentWeapon is Two-Handed.
	 * Weapon Blocking is possible only when the CurrentWeapon is Two-Handed.
	 * Animations and Blendspaces will be changed also.
	 * Will be set in the Weapon's Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items | TwoHanded")
	bool bIsTwoHanded;

	//Name of the socket the weapon will get attached to when sheathed
	FName SheathSocketName;

	//Name of the socket the weapon will get attached to when drawn
	FName HandSocketName;

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

	UFUNCTION(BlueprintCallable)
	void Sheath(AMain* Char);

	UFUNCTION(BlueprintCallable)
	void Unsheathe(AMain* Char);
	
	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
	FORCEINLINE void SetInstigator(AController* Instigator) { WeaponInstigator = Instigator; }
};
