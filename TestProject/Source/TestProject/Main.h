// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	EPS_UnarmedIdle UMETA(DisplayName = "UnarmedIdle"),
	EPS_CombatUnarmed UMETA(DisplayName = "CombatUnarmed"),
	EPS_ShieldUnarmed UMETA(DisplayName = "ShieldUnarmed"),	// todo remove this; merge with CombatArmed

	EPS_CombatArmed_1HM UMETA(DisplayName = "CombatArmed_OneHandedMelee"),
	EPS_CombatArmed_2HM UMETA(DisplayName = "CombatArmed_TwoHandedMelee"),
	EPS_CombatArmed_Bow UMETA(DisplayName = "CombatArmed_Bow"),

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

/** The types of Physical surfaces in the game. */
UENUM(BlueprintType)
enum class EPhysicalMaterials : uint8
{
	EPM_NormalLand UMETA(DisplayName = "Normal Land"),
	EPM_GrassyLand UMETA(DisplayName = "Grassy Land"),
	EPM_Stone UMETA(DisplayName = "Stone"),
	EPM_Wood UMETA(DisplayName = "Wood"),
	EPM_Water UMETA(DisplayName = "Water"),

	EPM_MAX UMETA(DisplayName = "MAX")
};

UCLASS()
class TESTPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

protected:
	/**Camera boom positioning the camera behind the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Following Camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Player Inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* InventoryComponent;

	/** The Weapon that the Character is currently using */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquippedWeapon;

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

	/**Base turn rates to scale turning function for the camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bBlocking;

	// States if the Character is currently aiming the Bow or not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bIsAimingBow;

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

	/** Array of footstep sounds for each type of Physical material. Values are set in Blueprints.
	*	Each Physical material has `NumberOfFootstepSounds` sounds.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FootstepSounds")
	TArray<USoundCue*> RunningFootstepSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FootstepSounds")
	TArray<USoundCue*> SprintingFootstepSounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bInCombatMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

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


	/**  End of Player Stats	*/

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	// The random number generated by GetRandomIdleAnimSlot(), to be used in the Blend Poses by Int block.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	int32 IdleAnimSlot;

private:

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

	/** E key button */
	bool bEKeyDown;

	/** C key button */
	bool bCKeyDown;

	// States if the Character is currently performing a Melee Attack or not.
	bool bIsMeleeAttacking;

	/** Sections of the Combat Montage that will play when the Player attacks. */
	int32 AttackComboSection;

	const int32 NumberOfMeleeAttacks = 3;
	const int32 NumberOfMeleeComboAttacks = 3;

	/** The SwingSounds TArray Index that will increase as the number of swings increases.
	 * This will help emit different sounds for each swing of the Combo Attacks.
	 * Gets reset every time AttackEnd gets called and increments each time PlaySwingSound gets called.
	 */
	int32 SwingSoundIndex;

	/** If the Player Attacks multiple times within this time frame, they will perform Combo Attacks.
	 *	Once this time is reached(3s), the attacks will reset back to normal.*/
	const float AttackComboSectionResetTime = 1.0f;


	TArray<FName> HitSocketNames;

	/** Used to make the Player reset back to the first attack animation if a timer limit(3s) is reached. */
	FTimerHandle AttackTimerHandle;


	//Location of the Weapon about to be picked up by the Player. The currently equipped weapon will be placed at this location.
	FVector OverlappingWeaponLocation;

	/** Time limit if reached, will make the timer handle call IdleEnd().*/
	const float IdleTimeLimit = 20.0f;

	/** Used to choose a random idle animation of the player(by calling IdleEnd()),
	 *	if the Player has not moved in IdleTimeLimit time. */
	FTimerDelegate TimerDel;
	FTimerHandle IdleAnimTimerHandle;


	TArray<int32> NumberOfIdleAnims;

	// A random number between [0 -> NumberOfFootstepSounds] will decide which footstep sound should be used.
	const int32 NumberOfFootstepSounds = 5;
	// TArray<int32> NumberOfFootstepSounds;


protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:

	// Sets default values for this character's properties
	AMain();

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

	/** C Key */
	void CKeyUp();
	void CKeyDown();

	void CheckPlayerStatus();
	
	void CheckStaminaStatus(float DeltaTime);


	/** Make the camera smoothly center back to its position*/
	UFUNCTION(BlueprintImplementableEvent)
	void CenterCamera(const FRotator& InCenterRotation);

	/** Make the camera zoom out slightly when sprinting */
	UFUNCTION(BlueprintImplementableEvent)
	void SprintStartCameraZoomOut();

	/** Make the camera return back to its original position after sprinting stops. */
	UFUNCTION(BlueprintImplementableEvent)
	void SprintEndCameraZoomIn();

	/** Make the camera zoom out and in smoothly when the Character goes into Combat Mode. */

	// Smoothly move the camera away from the Character.
	UFUNCTION(BlueprintImplementableEvent)
	void CombatModeCameraZoomOut(const FVector& FinalPosition);
	// Smoothly move the camera towards the Character.
	UFUNCTION(BlueprintImplementableEvent)
	void NormalModeCameraZoomIn();

	/** Make the Character crouch smoothly using Blueprints. */

	UFUNCTION(BlueprintImplementableEvent, Category = "Crouching")
	void PlayerCrouch();

	UFUNCTION(BlueprintImplementableEvent, Category = "Crouching")
	void PlayerUnCrouch();

	UFUNCTION(BlueprintCallable, Category = "Crouching")
	void CrouchEnd();


	void LeaveCombatMode();

	void EnterCombatMode();

	// Make the camera smoothly zoom in to the right of the Character when aiming with the bow
	UFUNCTION(BlueprintImplementableEvent, Category = "Bow")
	void BowAimingCameraZoomIn();

	// Make the camera smoothly zoom out to its original position.
	UFUNCTION(BlueprintImplementableEvent, Category = "Bow")
	void BowAimingCameraZoomOut();
	
	void ReloadBow();

	// Fire an arrow from the Bow.
	void BowAttack();

	// Called from an Anim Notify at the end of the Bow Firing Animation.
	void BowAttackEnd();

	// Sets bIsAimingBow to true. Called by an Anim Notify at the end of the "Drawing Arrow" animation.
	UFUNCTION(BlueprintCallable)
	void StartAimingBow();

	/** Set bInterpToEnemy
	*	@param Interp true or false depending on proximity to the Enemy
	*/
	void SetInterpToEnemy(bool Interp);

	//int32 GetRightHandAttackSection();

	UFUNCTION()
	void ResetMeleeAttackComboSection();

	void MeleeAttack();

	void PlayMeleeAttack(int32 Section);

	UFUNCTION(BlueprintCallable)
	void MeleeAttackEnd();

	void Block();

	UFUNCTION(BlueprintCallable)
	void BlockEnd();

	void Impact(int32 Section);

	void PlayBlockImpactAnimation(int32 Section);


	//getters

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE AMainPlayerController* GetMainPlayerController() const { return MainPlayerController; }

	FORCEINLINE AEnemy* GetCombatTarget() const { return CombatTarget; }

	FORCEINLINE bool GetbBlocking() const { return bBlocking; }

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE AShield* GetEquippedShield() const { return EquippedShield; }

	FORCEINLINE float GetStamina() const { return Stamina; }


	//setters

	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	FORCEINLINE void SetPlayerStatus(EPlayerStatus State) { if (PlayerStatus != State)PlayerStatus = State; }

	FORCEINLINE void SetbBlocking(bool Blocking) { bBlocking = Blocking; }

	void SetEquippedWeapon(AWeapon* WeaponToSet);

	void SetEquippedShield(AShield* ShieldToSet);

	FORCEINLINE void SetActiveOverlappingItem(AItem* ItemToSet) { ActiveOverlappingItem = ItemToSet; }

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FORCEINLINE void SetbHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	void SetIsAttacking(bool Attacking);

	FORCEINLINE void SetbInterpToEnemy(bool InterpToEnemy) { bInterpToEnemy = InterpToEnemy; }

	FORCEINLINE void SetStamina(float MainStamina) { Stamina = MainStamina; }


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

	void PlayHitSound();

	/**
	 * Line-trace from the Character's location to the ground to find out the Physical Material
	 * of the surface it is on to play a footstep sound.
	 * 
	 * Called by an Anim Notify in the Character's Walking or Running animation. 
	 */
	UFUNCTION(BlueprintCallable)
	void Footstep();

	void PlayFootstepSound(EPhysicalMaterials& PhysicalMaterial, FVector& LocationToPlayAt);

	FORCEINLINE bool GetbIsWeaponDrawn() const { return bIsWeaponDrawn; }

	FORCEINLINE void SetbIsWeaponDrawn(bool IsWeaponDrawn) { bIsWeaponDrawn = IsWeaponDrawn; }

	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	FORCEINLINE void SetInCombatMode(bool InCombatMode) { bInCombatMode = InCombatMode; }

	FORCEINLINE bool GetInCombatMode() const { return bInCombatMode; }

	FORCEINLINE bool GetIsAttacking() const { return (bIsMeleeAttacking || bIsAimingBow); }
};
