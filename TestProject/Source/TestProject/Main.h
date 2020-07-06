// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	EPS_UnarmedIdle UMETA(DisplayName = "UnarmedIdle"),
	EPS_UnarmedCombat UMETA(DisplayName = "UnarmedCombat"),
	EPS_ShieldUnarmed UMETA(DisplayName = "ShieldUnarmed"),
	EPS_CombatArmed_1H UMETA(DisplayName = "CombatArmed_1H"),
	EPS_CombatArmed_2H UMETA(DisplayName = "CombatArmed_2H"),

	EPS_MAX UMETA(DisplayName = "DefaultMAX")
};

/** The Character's states of movement*/
UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

/** The Character's Stamina States. Used for reducing stamina when sprinting. */
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


	/**Camera boom positioning the camera behind the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Following Camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	// Sets default values for this character's properties
	AMain();

	/** Player Inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* Inventory;

	/** The Weapon that the Character is currently using */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	bool bIsWeaponDrawn;

	/** The Shield that the Character is currently using */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AShield* EquippedShield;

	/** Gets set to the current Item whose CollisionVolume is currently being overlapped by the Character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	/** Location of the current Combat Target */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EPlayerStatus PlayerStatus;

	/** Rate at which Stamina drains when the Character is sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	/** Minimum amount of Stamina above which the Stamina Status can be set to ESS_Normal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintingSpeed;

	float CombatMaxWalkSpeed;

	float CombatSprintingSpeed;

	float BlockingMaxWalkSpeed;

	float CrouchedMaxWalkSpeed;

	float CrouchedCombatMaxWalkSpeed;

	bool bShiftKeyDown;

	/** Player Movement*/

	bool bMovingForward;

	bool bMovingRight;

	bool bJumping;


	/**Base turn rates to scale turning function for the camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** For turning towards the current Combat Target */
	bool bInterpToEnemy;

	float InterpSpeed;

	/** LMB: Left Mouse Button*/
	bool bLMBDown;

	/** RMB: Right Mouse Button*/
	bool bRMBDown;

	/** ESC: Escape Button*/
	bool bESCDown;

	/** F: F Button*/
	bool bRKeyDown;

	/** Ctrl button*/
	bool bCtrlDown;

	/** Tab button */
	bool bTabDown;

	/** E key */
	bool bEKeyDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bInCombatMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bCrouched;

	/** Sections of the Combat Montage that will play when the Player attacks. */
	int32 AttackComboSection;

	/** The SwingSounds TArray Index that will increase as the number of swings increases.
	 * This will help emit different sounds for each swing of the Combo Attacks.
	 * Gets reset every time AttackEnd gets called and increments each time PlaySwingSound gets called.
	 */
	int32 SwingSoundIndex;

	/** If the Player Attacks multiple times within this time frame, they will perform Combo Attacks.
	 *	Once this time is reached(3s), the attacks will reset back to normal.*/
	const float AttackComboSectionResetTime = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bBlocking;

	/** Anim Montage for Attacks*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	/** Anim Montage for Sheathing, Drawing and Impacts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* UpperBodyMontage;

	/** Particles emitted when the player gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	/** Sound that gets emitted when the player gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat ")
	class USoundCue* HitSound;

	TArray<FName> HitSocketNames;

	/** Used to make the Player reset back to the first attack animation if a timer limit(3s) is reached. */
	FTimerHandle AttackTimerHandle;
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


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	/** TSubclass that specifies to GetOverlappingActors()
	 *	to get overlapping actors of only type AEnemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	/** TSubclass that specifies to GetOverlappingActors()
	 *	to get overlapping actors of only type AActor.
	 *	(For the Interact function) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TSubclassOf<AActor> ActorFilter;

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	TSubclassOf<class AItemStorage> WeaponStorage;

	//Location of the Weapon about to be picked up by the Player. The currently equipped weapon will be placed at this location.
	FVector OverlappingWeaponLocation;

	/** Time limit if reached, will make the timer handle call IdleEnd().*/
	const float IdleTimeLimit = 10.0f;

	/** Used to choose a random idle animation of the player(by calling IdleEnd()),
	 *	if the Player has not moved in IdleTimeLimit time. */
	FTimerDelegate TimerDel;
	FTimerHandle IdleAnimTimerHandle;

	// The random number generated by GetRandomIdleAnimSlot(), to be used in the Blend Poses by Int block.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	int32 IdleAnimSlot;

	TArray<int32> NumberOfIdleAnims;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:
	// Called every frame
	void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called to check certain conditions before allowing Player movement */
	bool bCanMove(float Value);
	void ResetIdleTimer();

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

	/** RMB: Right Mouse Button*/
	void RMBUp();
	void RMBDown();

	/** ESC: Escape Button*/
	void ESCUp();
	void ESCDown();

	/** F: F Button*/
	void RKeyUp();
	void RKeyDown();

	/** Ctrl Key*/
	void CtrlUp();
	void CtrlDown();

	/** Tab key */
	void TabUp();
	void TabDown();

	/** E Key */
	void EKeyUp();
	void EKeyDown();


	/** Make the Character crouch smoothly using Blueprints. */

	UFUNCTION(BlueprintImplementableEvent, Category = "Crouching")
	void PlayerCrouch();

	UFUNCTION(BlueprintImplementableEvent, Category = "Crouching")
	void PlayerUnCrouch();

	UFUNCTION(BlueprintCallable, Category = "Crouching")
	void CrouchEnd();
	
	void CheckPlayerStatus();

	void SheatheWeapon();

	// Called in BP so that the weapon gets attached to the Sheath at the right moment in the Sheathe animation.
	UFUNCTION(BlueprintCallable)
	void TimedSheathe();

	void DrawWeapon();

	// Called in BP so that the weapon gets attached to the hand at the right moment in the Draw animation.
	UFUNCTION(BlueprintCallable)
	void TimedDraw();

	/** Set bInterpToEnemy
	*	@param Interp true or false depending on proximity to the Enemy
	*/
	void SetInterpToEnemy(bool Interp);

	void PlayMeleeAttack(int32 Section);

	UFUNCTION()
	void ResetMeleeAttackComboSection();

	void MeleeAttack();

	UFUNCTION(BlueprintCallable)
	void MeleeAttackEnd();

	void Block();

	UFUNCTION(BlueprintCallable)
	void BlockEnd();

	void Impact(int32 Section);

	void BlockImpact(int32 Section);


	//getters

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }


	//setters

	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	FORCEINLINE void SetPlayerStatus(EPlayerStatus State) { PlayerStatus = State; }

	void SetCurrentWeapon(AWeapon* WeaponToSet);

	void SetEquippedShield(AShield* ShieldToSet);

	FORCEINLINE void SetActiveOverlappingItem(AItem* ItemToSet) { ActiveOverlappingItem = ItemToSet; }

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }


	/** Called if health of player becomes less than equal to zero */
	void Die();

	/** Called to increment coin count of player if they pick up a coin/coins */
	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);

	/** Called to increase health of player if they comsume an Item that increases Health */
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	/** Takes the location of the target and returns how 
		*	much the player has to rotate to orient it to the target. */
	FRotator GetLookAtRotationYaw(FVector Target);

	/** Pressed down to enable sprinting*/
	void ShiftKeyDown();

	/** Released to stop sprinting*/
	void ShiftKeyUp();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();


	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	                 class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void Jump() override;

	void UpdateCombatTarget();

	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	void LoadGameNoSwitch();

	bool CanCheckStaminaStatus();

	/**
	 *	This function will determine which Idle animation out of the set of Idle animations will be set.
	 *	@param PlayerStatusNo - Number signifying what state the Player is curently in.
	 *
	 *							0 - IdleUnarmed
	 *							1 - CombatUnarmed
	 *							2 - CombatArmed	- 1Handed
	 *							3 - CombatArmed - 2Handed
	 *
	 *	Then, NumberOfIdleAnims[PlayerStatusNo] is used as the upper limit of the RandRange function, which will return a
	 *	random number from 0 to (NumberOfIdleAnims[PlayerStatusNo] - 1).
	 *	
	 *	The Blend Poses by Int block in the Main Anim_BP will then choose the Idle Animation connected
	 *	to the random number's slot.
	 */

	UFUNCTION(BlueprintCallable)
	void IdleEnd(int32 PlayerStatusNo);

	void SpawnHitParticles(AEnemy* DamageCauser);

	//UFUNCTION(BlueprintCallable, Category = "Items")
	//void UseItem(class AItem* Item);
};
