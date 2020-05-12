// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal		UMETA(DisplayName = "Normal"),
	EMS_Sprinting	UMETA(DisplayName = "Sprinting"),
	EMS_Dead		UMETA(DisplayName = "Dead"),

	EMS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class TESTPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
		class AMainPlayerController* MainPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		FVector CombatTargetLocation;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Player Movement*/

	
	bool bMovingForward;

	bool bMovingRight;

	/**Camera boom positioning the camera behind the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Following Camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/**Base turn rates to scale turning function for the camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/** LMB: Left Mouse Button*/
	bool bLMBDown;

	/** ESC: Escape Button*/
	bool bESCDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
		bool bAttacking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* CombatMontage;

	/** Particles emitted when the player gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UParticleSystem* HitParticles;

	/** Sound that gets emitted when the player gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat ")
		class USoundCue* HitSound;

	/**
	*
	*	Player Stats
	*
	*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	int32 Coins;

	float InterpSpeed;

	bool bInterpToEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	/** TSubclass that specifies to GetOverlappingActors()
	 *	to get overlapping actors of only type AEnemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat ")
		TSubclassOf<AEnemy>EnemyFilter;

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
		TSubclassOf<class AItemStorage> WeaponStorage;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called to check certain conditions before allowing Player movement */
	bool bCanMove(float Value);

	/** Called for front and back movement*/
	void MoveForward(float Value);

	/**Called for left and right player rotation (Yaw rotation)*/
	void Turn(float Value);

	/** Called for up and down player rotation (Pitch rotation)*/
	void LookUp(float Value);

	/**Called for left and right movement*/
	void MoveRight(float Value);

	/** Called via input to turn at a given rate
	* @param Rate This is normalized, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to turn up/down at a given rate
	* @param Rate This is normalized, i.e. 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	/** LMB: Left Mouse Button*/
	void LMBUp();
	void LMBDown();

	/** ESC: Escape Button*/
	void ESCUp();
	void ESCDown();

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	//getters

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Takes the location of the target and returns how 
	*	much the player has to rotate to orient it to the target. */
	FRotator GetLookAtRotationYaw(FVector Target);

	//setters

	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	void SetEquippedWeapon(AWeapon* WeaponToSet);

	FORCEINLINE void SetActiveOverlappingItem(AItem* ItemToSet) { ActiveOverlappingItem = ItemToSet; }

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }


	/** Called to decrease health of player if they take damage */
	void DecrementHealth(float Amount);

	/** Called if health of player becomes lesss than equal to zero */
	void Die();

	/** Called to increment coin count of player if they pick up a coin/coins */
	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);	

	/** Called to increase health of player if they take pick up a health potion */
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	/** Pressed down to enable sprinting*/
	void ShiftKeyDown();

	/** Released to stop sprinting*/
	void ShiftKeyUp();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	/** Set bInterpToEnemy
	*	@param Interp true or false depending on proximity to the Enemy
	*/
	void SetInterpToEnemy(bool Interp);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
							class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	virtual void Jump() override;

	void UpdateCombatTarget();

	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);
};
