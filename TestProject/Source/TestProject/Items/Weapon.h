// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Pickup UMETA(DisplayName = "Pickup"),
	EWS_Sheathed UMETA(DisplayName = "Sheathed"),
	EWS_Drawn UMETA(DisplayName = "Drawn"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_OneHandedMelee UMETA(DisplayName = "OneHanded_Melee"),
	EWT_TwoHandedMelee UMETA(DisplayName = "TwoHanded_Melee"),
	EWT_Bow UMETA(DisplayName = "Bow"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
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


protected:

	void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Enums")
	EWeaponState WeaponState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponProperties | Enums")
	EWeaponType WeaponType = EWeaponType::EWT_OneHandedMelee;

	/**If true, the Player will treat the weapon as two handed.
	 * No Shield can be equipped if the EquippedWeapon is Two-Handed.
	 * Weapon Blocking is possible only when the EquippedWeapon is Two-Handed.
	 * Animations and Blendspaces will be changed also.
	 * Will be set in the Weapon's Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Combat")
	bool bIsTwoHanded;

	/** To choose if the weapon's particle effects should show even after equipping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Particles")
	bool bWeaponParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Sounds")
	class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Sounds")
	USoundCue* OnSheathSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeletal Mesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties | Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponProperties | Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponProperties | Combat")
	AController* WeaponInstigator;

	// Name of the socket the weapon will get attached to when sheathed
	FName SheathSocketName;

	// Name of the socket the weapon will get attached to when drawn
	FName HandSocketName;

	// Prefix to be added to the name of the Animation to be played in the Combat Montage.
	FString AttackAnimPrefix;

	// Name of the Sheath Animation to be played in the UpperBody Montage.
	FName SheathAnimName;

	// Name of the Draw Animation to be played in the UpperBody Montage.
	FName DrawAnimName;

public:

	bool UseItem(AMain* Main) override;

	/**Called if an Overlap Event starts. */
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	/** Called when the Overlap Event ends. */
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex) override;


	void Equip(class AMain* Char);

	//UFUNCTION(BlueprintCallable)
	//void Sheath(AMain* Char);

	//UFUNCTION(BlueprintCallable)
	//void Unsheathe(AMain* Char);

	// getters

	FORCEINLINE bool IsTwoHanded() const { return bIsTwoHanded; }

	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }

	FORCEINLINE FName GetSheathSocketName() const { return SheathSocketName; }

	FORCEINLINE FName GetHandSocketName() const { return HandSocketName; }

	FORCEINLINE FString GetAttackAnimPrefix() const { return AttackAnimPrefix; }

	FORCEINLINE FName GetSheathAnimName() const { return SheathAnimName; }

	FORCEINLINE FName GetDrawAnimName() const { return DrawAnimName; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	// setters

	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }

	FORCEINLINE void SetInstigator(AController* Weapon_Instigator) { WeaponInstigator = Weapon_Instigator; }

	void PlaySheathSound() const;
	void PlayDrawSound() const;

	// Called in BP so that the weapon gets attached to the hand at the right moment in the Draw animation.
	UFUNCTION(BlueprintCallable)
	void TimedDraw(AMain* Main);

	// Called in BP so that the weapon gets attached to the Sheath at the right moment in the Sheathe animation.
	UFUNCTION(BlueprintCallable)
	void TimedSheathe(AMain* Main);
};
