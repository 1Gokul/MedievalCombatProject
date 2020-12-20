//  Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"


#include "Items/Weapon.h"
#include "Items/Shield.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "GameSave.h"
#include "ItemStorage.h"
#include "MainAnimInstance.h"
#include "InventoryComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Items/MeleeWeapon.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


//  Sets default values
AMain::AMain()
{
	//  Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm (pulls towards player)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), FName("CameraBoomAttachSocket")); // Attach to root component
	CameraBoom->TargetArmLength = 150.0f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Set size of collision capsule
	GetCapsuleComponent()->SetCapsuleSize(36.0f, 92.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of spring arm 
	FollowCamera->bUsePawnControlRotation = false; // Will stay fixed to CameraBoom and will not rotate

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	InventoryComponent->NumberOfSlots = 42;

	// Set out turn rates for input
	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;

	// To make sure the player doesn't rotate when the controller rotates and only the camera does.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	// Configure character movement
	//GetCharacterMovement()->bOrientRotationToMovement = true; // Character rotates to direction of input.
	GetCharacterMovement()->RotationRate = FRotator(0.0, 1000.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 650.0f;
	GetCharacterMovement()->AirControl = 0.20f;


	//  Player Stats
	MaxHealth = 100.0f;
	Health = 65.0f;
	MaxStamina = 150.0f;
	Stamina = 120.0f;
	Coins = 0;

	RunningSpeed = 600.0f;
	SprintingSpeed = 850.0f;

	CombatMaxWalkSpeed = 450.0f;
	CombatSprintingSpeed = 700.0f;
	BlockingMaxWalkSpeed = 150.0f;
	CrouchedMaxWalkSpeed = 200.0f;
	CrouchedCombatMaxWalkSpeed = 150.0f;
	BowAimingMaxWalkSpeed = 150.0f;
	BowAim_TargetArmLength = 150.0f;

	OverlappingWeaponLocation = FVector(0.0, 0.0, 0.0);

	BowAim_LastYawRotation = 0.0f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bRMBDown = false;
	bRKeyDown = false;
	bCtrlDown = false;
	bTabDown = false;
	bEKeyDown = false;
	bInCombatMode = false;
	bIsMeleeAttacking = false;
	bIsDrawingArrow = false;
	bIsTurningWhileAiming = false;
	bBlocking = false;
	bCrouched = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
	bJumping = false;
	bInterpToEnemy = false;
	bIsWeaponDrawn = false;

	AttackComboSection = 0;
	SwingSoundIndex = 0;
	IdleAnimSlot = 0;

	PlayerStatus = EPlayerStatus::EPS_UnarmedIdle;

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.0f;
	MinSprintStamina = 50.0f;

	InterpSpeed = 15.0f;

	//  Sockets in the Character's Skeleton to emit Blood when hit.
	HitSocketNames.Add("BodyFrontHitSocket");
	HitSocketNames.Add("HeadFrontHitSocket");
	HitSocketNames.Add("BodyRearHitSocket");
	HitSocketNames.Add("HeadRearHitSocket");

	// Number of Idle Animations for each state
	NumberOfIdleAnims.Add(6); // UnarmedNormal
	NumberOfIdleAnims.Add(6); // UnarmedCombat
	NumberOfIdleAnims.Add(6); // ShieldOnly
	NumberOfIdleAnims.Add(5); // OneHandedMelee
	NumberOfIdleAnims.Add(4); // TwoHandedMelee
	NumberOfIdleAnims.Add(4); // Bow

	// Delegate calls IdleEnd after Timer reaches zero, when Player is in 
	TimerDel.BindUFunction(this, FName("IdleEnd"), static_cast<int32>(PlayerStatus));
}

//  Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	// If the level is not the first level,
	if (MapName != "TestMap")
	{
		// Load all items of player after level transition
		LoadGameNoSwitch();

		// Save the game.
		SaveGame();
	}

	// Return to game mode if game was loaded from the pause menu.
	if (MainPlayerController)
	{
		MainPlayerController->GameModeOnly();
	}

	// Display the HUD
	MainPlayerController->DisplayHUD();
}

//  Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	CheckPlayerStatus();

	CheckStaminaStatus(DeltaTime);

	if (bInterpToEnemy && CombatTarget)
	{
		const FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		const FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		// Rotate to be directly facing the current Combat Target.
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();

		if (MainPlayerController)
		{
			MainPlayerController->SetEnemyLocation(CombatTargetLocation);
		}
	}

	// If IdleTimer is not active and Player is not crouched
	if (!GetWorldTimerManager().IsTimerActive(IdleAnimTimerHandle) && !bCrouched && !bIsDrawingArrow)
	{
		// If an Idle Animation is not already playing AND the Player is not moving
		if (IdleAnimSlot == 0
			&& (Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance())->GetMovementSpeed() == 0.0f))
		{
			// Start the Timer and set it for IdleTimeLimit seconds.
			GetWorldTimerManager().SetTimer(IdleAnimTimerHandle, TimerDel, IdleTimeLimit, false);
		}
	}

	if (bIsDrawingArrow || bIsBowAimed)
	{
		/*float ControlRotationYaw = GetControlRotation().Yaw;

		if(ControlRotationYaw < 0)ControlRotationYaw -= 360.0f;*/

		/*float ActorRotationYaw = GetActorRotation().Yaw;

		if (ActorRotationYaw < 0)ActorRotationYaw += 360.0f;

		float ControlRotationYaw = GetControlRotation().Yaw - ActorRotationYaw;

		if(ControlRotationYaw < -180)ControlRotationYaw += 360.0f;
		else ControlRotationYaw += 60; */


		/*UE_LOG(LogTemp, Warning, TEXT("%f -  %f = %f"), ControlRotationYaw, ActorRotationYaw,
		       (ControlRotationYaw - ActorRotationYaw));*/

		//// If the Character is going to rotate its Yaw beyond 90 degrees, rotate them 90 degrees to the right.
		//if (FMath::Abs(ControlRotationYaw - ActorRotationYaw) > 90.0f)
		//{
		//	/*SetActorRotation(FRotator(0.0f, ControlRotationYaw, 0.0f));*/

		//// Play the 'TurnRight90' Animation
		//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		//if (AnimInstance && BowAimTurningMontage)
		//{
		//	bIsTurningWhileAiming = true;
		//	
		//	AnimInstance->Montage_Play(BowAimTurningMontage, 1.0f);

		//	AnimInstance->Montage_JumpToSection(FName("TurnRight90"), BowAimTurningMontage);
		//}
		//}

		//// else, set the Control Rotation and make sure both Pitch and Yaw fall in the range (-90deg, 90deg). 
		//else
		//{
		/*ControlRotationForBow = GetControlRotation();

		if(ControlRotationForBow.Pitch > 270.0f)ControlRotationForBow.Pitch -= 360.0f;
		if(ControlRotationForBow.Yaw > 270.0f)ControlRotationForBow.Yaw -= 360.0f;*/
		//}


		ControlRotationForBow = GetControlRotation();
		if (ControlRotationForBow.Pitch > 270.0f)ControlRotationForBow.Pitch -= 360.0f;
		//if(ControlRotationForBow.Yaw > 270.0f)ControlRotationForBow.Yaw -= 360.0f;

		//if(ControlRotationForBow.Yaw - BowAim_LastYawRotation > 30.0f)
		//{
		//	// Play the 'TurnRight90' Animation
		//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		//	if (AnimInstance && BowAimTurningMontage)
		//	{
		//		bIsTurningWhileAiming = true;
		//		
		//		AnimInstance->Montage_Play(BowAimTurningMontage, 2.0f);

		//		AnimInstance->Montage_JumpToSection(FName("TurnRight90"), BowAimTurningMontage);
		//	}
		//}

		//ControlRotationForBow.Yaw -= GetActorRotation().Yaw;
	}
	/*else
	{
		ControlRotationForBow = FRotator(0.0f, 0.0f, 0.0f);
	}*/
}

//  Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMB", IE_Released, this, &AMain::RMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);


	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

	PlayerInputComponent->BindAction("CombatMode", IE_Pressed, this, &AMain::RKeyDown);
	PlayerInputComponent->BindAction("CombatMode", IE_Released, this, &AMain::RKeyUp);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMain::CtrlDown);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMain::CtrlUp);

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AMain::TabDown).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("Inventory", IE_Released, this, &AMain::TabUp).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::EKeyDown).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMain::EKeyUp).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("ResetCamera", IE_Pressed, this, &AMain::CKeyDown);
	PlayerInputComponent->BindAction("ResetCamera", IE_Released, this, &AMain::CKeyUp);
}

bool AMain::bCanMove(float Value)
{
	if (MainPlayerController)
	{
		return (
			(Value != 0.0f) // If Value to move is 0
			&& (!bIsMeleeAttacking) // If Attacking
			&& (MovementStatus != EMovementStatus::EMS_Dead) // If not Dead
			&& (!MainPlayerController->IsAnyMenuVisible()) // If Inventory Menu Visible
		);
	}
	return false;
}

void AMain::ResetIdleTimer()
{
	// If the HUD is not currently visible, display it
	if (!MainPlayerController->IsHUDVisible())
	{
		MainPlayerController->DisplayHUD();
	}

	// Reset the Idle Anim Slot
	IdleAnimSlot = 0;

	// Pause IdleAnimTimerHandle if Active
	if (GetWorldTimerManager().IsTimerActive(IdleAnimTimerHandle))
	{
		GetWorldTimerManager().PauseTimer(IdleAnimTimerHandle);
	}
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;

	if (bCanMove(Value))
	{
		ResetIdleTimer();

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		/*if(bShiftKeyDown)
		{
			UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

			if(MainAnimInstance)
			{
				AddActorLocalRotation(FRotator(0.0f, MainAnimInstance->GetMovementSpeed(), 0.0f));
			}
		}*/

		bMovingForward = true;
	}
}


void AMain::MoveRight(float Value)
{
	bMovingRight = false;

	// If the Character can move AND is not sprinting(Because currently Strafing is not allowed while sprinting)
	if (bCanMove(Value) && !CanCheckStaminaStatus())
	{
		ResetIdleTimer();

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		/*if(bShiftKeyDown)
		{
			UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

			if(MainAnimInstance)
			{
				AddActorLocalRotation(FRotator(0.0f, MainAnimInstance->GetMovementSpeed(), 0.0f));
			}
		}*/

		bMovingRight = true;
	}
}

void AMain::Turn(float Value)
{
	if (bCanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}

void AMain::LookUp(float Value)
{
	if (bCanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}


void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * (GetWorld()->GetDeltaSeconds()));
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * (GetWorld()->GetDeltaSeconds()));
}

void AMain::LMBUp()
{
	bLMBDown = false;

	if (bIsDrawingArrow)
	{
		AbortBowAiming();
	}
	else if (bIsBowAimed)
	{
		BowAttack();
	}
}


void AMain::LMBDown()
{
	bLMBDown = true;

	ResetIdleTimer();

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (MainPlayerController)
	{
		if (MainPlayerController->IsPauseMenuVisible())return;
	}


	//// If Player is overlapping with an Item, they can equip it 
	//if (ActiveOverlappingItem)
	//{
	//	// If the Item is a Weapon
	//	AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
	//	if (Weapon)
	//	{
	//		if (EquippedWeapon)
	//		{
	//			if (bIsWeaponDrawn)
	//			{
	//				OverlappingWeaponLocation = Weapon->GetActorLocation();

	//				Weapon->Equip(this);
	//				SetActiveOverlappingItem(nullptr);
	//			}
	//		}
	//		else
	//		{
	//			OverlappingWeaponLocation = Weapon->GetActorLocation();

	//			Weapon->Equip(this);
	//			SetActiveOverlappingItem(nullptr);
	//		}
	//	}
	//	else
	//	{
	//		AShield* Shield = Cast<AShield>(ActiveOverlappingItem);
	//		if (Shield)
	//		{
	//			Shield->Equip(this);
	//			SetActiveOverlappingItem(nullptr);
	//		}
	//	}
	//}

	/** if Player already has a weapon equipped AND
	*	is not already blocking, perform an Attack.
	*/

	// todo make all of these checks into one function
	if (!bBlocking)
	{
		UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());
		if (MainAnimInstance)
		{
			// If Player is not currently falling
			if (!MainAnimInstance->IsInAir())
			{
				if (bInCombatMode)
				{
					if (EquippedWeapon && bIsWeaponDrawn)
					{
						// If the Weapon is a Bow
						if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Bow)
						{
							ReloadBow();
						}
							// Else if the Weapon is a Melee Weapon
						else
						{
							MeleeAttack();
						}
					}
					else
					{
						MeleeAttack();
					}
				}
				else EnterCombatMode();
			}
		}
	}
}

void AMain::RMBUp()
{
	bBlocking = false;
	bRMBDown = false;

	// GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	// EquippedShield->DeactivateCollision();
}

void AMain::RMBDown()
{
	bRMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (MainPlayerController)
	{
		if (MainPlayerController->IsPauseMenuVisible())return;
	}

	/** else if Player already has a shield equipped AND
	*	is not already attacking, perform a Block.
	*/
	if (!bIsMeleeAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		ResetIdleTimer();

		// Blocking should be performed only with a Shield or a Two-Handed Melee Weapon.
		if ((EquippedWeapon && !EquippedShield && (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_TwoHandedMelee))
			|| (EquippedShield))
		{
			UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());
			if (MainAnimInstance)
			{
				// If Player is not currently falling
				if (!MainAnimInstance->IsInAir())
				{
					if (bInCombatMode)
					{
						if (!bBlocking)
						{
							bBlocking = true;
						}
					}
					else
					{
						EnterCombatMode();
					}
				}
			}
		}
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;


	if (MainPlayerController)
	{
		// If a UI Widget (Other than the HUD) is not currently active, display the Pause menu.
		if (!MainPlayerController->IsInventoryMenuVisible())
		{
			MainPlayerController->TogglePauseMenu();
		}
	}
}

void AMain::RKeyUp()
{
	bRKeyDown = false;
}


void AMain::RKeyDown()
{
	bRKeyDown = true;

	ResetIdleTimer();

	/** If already in Combat Mode, switch back to Normal Mode. */
	if (bInCombatMode && !bBlocking)
	{
		LeaveCombatMode();
	}

		/** If in Normal Mode, switch to Combat Mode. */
	else
	{
		EnterCombatMode();
	}
}

void AMain::CtrlUp()
{
	bCtrlDown = false;
}

void AMain::CtrlDown()
{
	bCtrlDown = true;

	ResetIdleTimer();

	if (GetCharacterMovement()->IsCrouching())
	{
		bCrouched = false;

		PlayerUnCrouch();

		GetCharacterMovement()->bOrientRotationToMovement = false; // Character rotates to direction of input.

		// GetCapsuleComponent()->SetCapsuleRadius(36.0);
	}
	else
	{
		bCrouched = true;

		PlayerCrouch();

		GetCharacterMovement()->bOrientRotationToMovement = true; // Character rotates to direction of input.

		// GetCapsuleComponent()->SetCapsuleRadius(60.0);
	}
}

void AMain::CrouchEnd()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMain::TabUp()
{
	bTabDown = false;
}

void AMain::TabDown()
{
	bTabDown = true;

	if (MainPlayerController)
	{
		// If a UI Widget (Other than the HUD) is not currently active, display the Inventory menu.
		if (!MainPlayerController->IsPauseMenuVisible())
		{
			MainPlayerController->ToggleInventoryMenu(InventoryComponent);
		}
	}
}

void AMain::EKeyUp()
{
	bEKeyDown = false;
}

void AMain::EKeyDown()
{
	bEKeyDown = true;

	ResetIdleTimer();

	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors, ActorFilter);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		//  if the OverlappingActor implements UInteractInterface
		if (OverlappingActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
		{
			// Execute Interact()
			Cast<IInteractInterface>(OverlappingActor)->Interact(this);

			break;
		}
	}
}

void AMain::CKeyUp()
{
	bCKeyDown = false;
}

void AMain::CKeyDown()
{
	bCKeyDown = true;

	CenterCamera(GetMesh()->GetComponentRotation().Add(0.0f, 90.0f, 0.0f));
}


void AMain::CheckPlayerStatus()
{
	if (bInCombatMode)
	{
		if (EquippedWeapon)
		{
			// Set the PlayerStatus depending on the Weapon Type
			switch (EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_OneHandedMelee: SetPlayerStatus(EPlayerStatus::EPS_CombatArmed_1HM);
				break;

			case EWeaponType::EWT_TwoHandedMelee: SetPlayerStatus(EPlayerStatus::EPS_CombatArmed_2HM);
				break;

			case EWeaponType::EWT_Bow: SetPlayerStatus(EPlayerStatus::EPS_CombatArmed_Bow);
				break;

			default: SetPlayerStatus((EPlayerStatus::EPS_CombatArmed_1HM));
				break;
			}
		}
		else
		{
			if (EquippedShield)SetPlayerStatus(EPlayerStatus::EPS_ShieldUnarmed);
			else SetPlayerStatus(EPlayerStatus::EPS_CombatUnarmed);
		}
	}
	else
	{
		SetPlayerStatus(EPlayerStatus::EPS_UnarmedIdle);
	}
}

void AMain::CheckStaminaStatus(float DeltaTime)
{
	//  Decrease in Stamina per second when sprinting.
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:

		if (CanCheckStaminaStatus())
		{
			// or blocking

			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}

			if ((bMovingForward || bMovingRight) && (!bBlocking) && (!GetCharacterMovement()->IsCrouching()))
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:

		if (CanCheckStaminaStatus())
		{
			if (Stamina - DeltaStamina <= 0.0f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				if ((bMovingForward || bMovingRight) && (!bBlocking) && (!GetCharacterMovement()->IsCrouching()))
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}
		else
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:

		if (CanCheckStaminaStatus())
		{
			Stamina = 0.0f;
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:

		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	default:
		break;
	}
}


void AMain::LeaveCombatMode()
{
	// If currently aiming, stop the Aiming Montage.
	if (bIsDrawingArrow || bIsBowAimed)
	{
		AbortBowAiming();
	}

	NormalModeCameraZoomIn();


	bInCombatMode = false;

	SetIsAttacking(false);

	// If a weapon is equipped, sheathe it.
	if (EquippedWeapon)
	{
		// Play the Sheath sound
		EquippedWeapon->PlaySheathSound();

		// Play the Sheath Animation
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);

			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetSheathAnimName(), UpperBodyMontage);
		}
	}
}

void AMain::EnterCombatMode()
{
	CombatModeCameraZoomOut();

	bInCombatMode = true;

	// If a weapon is equipped, draw it.
	if (EquippedWeapon)
	{
		// Play the Draw sound
		EquippedWeapon->PlayDrawSound();

		// Play the Draw Animation
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);

			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetDrawAnimName(), UpperBodyMontage);
		}
	}
}

void AMain::ReloadBow()
{
	bIsDrawingArrow = true;
	bUseControllerRotationYaw = true;

	// Zoom in
	BowAim_TargetArmLength = 100.0f;
	BowAimingCameraZoomIn();

	// Set BowAim_LastYawRotation to the current Yaw rotation
	BowAim_LastYawRotation = GetControlRotation().Yaw;

	// Play the Arrow Drawing Animation
	UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Play(BowAimMontage, 1.0f);

		// MainAnimInstance->Montage_JumpToSection(FName("DrawArrow"), UpperBodyMontage);
	}
}


void AMain::AbortBowAiming()
{
	UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

	// Stop playing the Arrow Drawing Animation
	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Stop(0.25f, BowAimMontage);

		// MainAnimInstance->Montage_JumpToSection(FName("DrawArrow"), UpperBodyMontage);
	}

	// Zoom out
	if (bInCombatMode)
	{
		BowAim_TargetArmLength = 400.0f;
		BowAimingCameraZoomOut();
	}

	bIsDrawingArrow = false;
	bIsBowAimed = false;
}

void AMain::BowAttack()
{
	bIsFiringArrow = true;

	UE_LOG(LogTemp, Warning, TEXT("BowAttack() has been called!"));

	// Play the Arrow Firing Animation
	UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Play(UpperBodyMontage, 1.0f);

		MainAnimInstance->Montage_JumpToSection(FName("FireArrow"), UpperBodyMontage);
	}

	// Zoom out
	BowAim_TargetArmLength = 400.0f;
	BowAimingCameraZoomOut();

	// bUseControllerRotationPitch = false;
}

void AMain::BowAttackEnd()
{
}

void AMain::ArrowDrawn_StartAiming()
{
	bIsDrawingArrow = false;
	bIsBowAimed = true;
}

void AMain::StopAimingBow()
{
	bIsBowAimed = false;
	bIsFiringArrow = false;
}

void AMain::BowAimingTurnFinished()
{
	//// Play the 'TurnRight90' Animation
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//if (AnimInstance && BowAimMontage)
	//{
	//	AnimInstance->Montage_Play(BowAimMontage, 1.0f);

	//	AnimInstance->Montage_JumpToSection(FName("Loop"), BowAimMontage);
	//}

	bIsTurningWhileAiming = false;
}


void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}


void AMain::ResetMeleeAttackComboSection()
{
	AttackComboSection = 0;
}

void AMain::MeleeAttack()
{
	ResetIdleTimer();

	if (!bIsMeleeAttacking && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		bIsMeleeAttacking = true;

		SetInterpToEnemy(true);

		/** AttackSection
		 *  == 0 OR 1 -> Normal Attack
		 *  == 2 -> Combo Attack
		*/
		PlayMeleeAttack((AttackComboSection++) % NumberOfMeleeAttacks);
	}
}


void AMain::PlayMeleeAttack(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();


	// Attack Sections start from 1
	Section += 1;


	if (AnimInstance && CombatMontage)
	{
		// Used for Enemy Hit Reaction
		AMeleeWeapon* MeleeWeapon = Cast<AMeleeWeapon>(EquippedWeapon);
		if (MeleeWeapon)MeleeWeapon->SetMainAttackSection(Section);


		FString AttackName;

		// Append Section number

		// If attack is using a weapon
		if (bIsWeaponDrawn)
		{
			AttackName.Append(EquippedWeapon->GetAttackAnimPrefix());

			if (!bCrouched)
			{
				// Get a random Final Combo Attack (Ranges from Attack_3 to Attack_5)
				if (Section == 3)Section += FMath::RandRange(0, NumberOfMeleeComboAttacks - 1);
			}
			else
			{
				// Only one Section for crouched attacks
				Section = 1;
				AttackName.Append("Crouched_");
			}
		}

			// else if unarmed melee attack
		else
		{
			// If Player has a Shield equipped, play the even-numbered attacks(right-handed attacks) only.
			if (PlayerStatus == EPlayerStatus::EPS_ShieldUnarmed)
			{
				// Override Section  todo Find a better way to find the even AttackSection
				Section = FMath::RandRange(2, 4);
				if (Section % 2)Section -= 1; // Set Section to play the 2nd Animation if RandRange returns 3.
			}

			AttackName.Append("UnarmedMeleeAttack_");
		}

		AttackName.AppendInt(Section);

		UE_LOG(LogTemp, Warning, TEXT("Attack = %s"), *AttackName);

		// Play Montage
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*AttackName, CombatMontage);
	}
}


void AMain::MeleeAttackEnd()
{
	bIsMeleeAttacking = false;

	SetInterpToEnemy(false);

	// Reset Swing Sound index
	SwingSoundIndex = 0;

	// Reset Combo Attack Section if Player does not press LMB within AttackComboSectionResetTime.
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMain::ResetMeleeAttackComboSection,
	                                AttackComboSectionResetTime);


	if (bLMBDown)
	{
		GetWorldTimerManager().ClearTimer(AttackTimerHandle);
		MeleeAttack();
	}
}

// NOT BEING USED
void AMain::Block()
{
	// if (MovementStatus != EMovementStatus::EMS_Dead) {

	// 	// Activate collision so that the MainShield can detect Overlaps.
	// 	EquippedShield->ActivateCollision();


	// 	if(!bBlocking)bBlocking = true;
	// Combat Montage method

	// // if Player was not already blocking
	// if (!bBlocking) {
	// 	bBlocking = true;

	// // Play the "Going To Block" Animation
	// UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// if (AnimInstance && CombatMontage) {

	// 	AnimInstance->Montage_Play(CombatMontage, 1.0f);
	// 	AnimInstance->Montage_JumpToSection(FName("BlockStart"), CombatMontage);

	// }
	// }
	// // If already blocking,
	// else{

	// 	// Play the "Blocking Idle" animation
	// 	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 	if (AnimInstance && CombatMontage) {
	// 			AnimInstance->Montage_Play(CombatMontage, 3.0f);
	// 			AnimInstance->Montage_JumpToSection(FName("BlockIdle"), CombatMontage);
	// 	}
	// }
	// }
}

// NOT BEING USED
void AMain::BlockEnd()
{
	// if (bRMBDown) {
	// 	Block();
	// }

	// else{
	// 	// Play the animation to go from blocking stance to normal
	// 	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 	if (AnimInstance && CombatMontage) {

	// 		AnimInstance->Montage_Play(CombatMontage, 3.0f);
	// 		AnimInstance->Montage_JumpToSection(FName("BlockEnd"), CombatMontage);

	// 	}

	// 	bBlocking = false;
	// }
}

void AMain::Impact(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && UpperBodyMontage)
	{
		ResetIdleTimer();

		FString HitName("Hit_");

		HitName.AppendInt(Section);

		// Play Montage
		AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*HitName, UpperBodyMontage);
	}
}


void AMain::PlayBlockImpactAnimation(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && UpperBodyMontage)
	{
		ResetIdleTimer();

		FString ImpactName("Impact_");

		ImpactName.AppendInt(Section);

		// Play Montage
		AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*ImpactName, UpperBodyMontage);
	}
}

void AMain::SetIsAttacking(bool Attacking)
{
	/*if (bIsMeleeAttacking != Attacking)bIsMeleeAttacking = Attacking;
	else if (bIsDrawingArrow != Attacking)bIsDrawingArrow = Attacking;*/

	bIsMeleeAttacking = bIsDrawingArrow = bIsBowAimed = false;
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 3.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
}

void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::IncrementHealth(float Amount)
{
	Health += Amount;

	if (Health >= MaxHealth)
	{
		Health = MaxHealth;
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.0f, LookAtRotation.Yaw, 0.0f);
	return LookAtRotationYaw;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;

	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		if (bInCombatMode)GetCharacterMovement()->MaxWalkSpeed = CombatSprintingSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else if (bInCombatMode)
	{
		if (GetCharacterMovement()->IsCrouching())GetCharacterMovement()->MaxWalkSpeedCrouched =
			CrouchedCombatMaxWalkSpeed;
		else if (EquippedWeapon && bIsDrawingArrow)GetCharacterMovement()->MaxWalkSpeed = BowAimingMaxWalkSpeed;
		else if (bBlocking)
		{
			GetCharacterMovement()->MaxWalkSpeed = BlockingMaxWalkSpeed;
		}
		else GetCharacterMovement()->MaxWalkSpeed = CombatMaxWalkSpeed;
	}

	else
	{
		if (GetCharacterMovement()->IsCrouching())GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedMaxWalkSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeaponToSet;
}


void AMain::SetEquippedShield(AShield* ShieldToSet)
{
	if (EquippedShield)
	{
		EquippedShield->Destroy();
	}

	EquippedShield = ShieldToSet;
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;

	if (CanCheckStaminaStatus())
	{
		SprintStartCameraZoomOut();
	}
}

void AMain::ShiftKeyUp()
{
	if (CanCheckStaminaStatus())
	{
		SprintEndCameraZoomIn();
	}

	bShiftKeyDown = false;
}

void AMain::PlaySwingSound()
{
	AMeleeWeapon* MeleeWeapon = Cast<AMeleeWeapon>(EquippedWeapon);

	if (MeleeWeapon)
	{
		// Each MeleeWeapon has specific Swing Sounds depending on the current Attack number.
		if (SwingSoundIndex < MeleeWeapon->SwingSounds.Num())
		{
			if (MeleeWeapon->SwingSounds[SwingSoundIndex])
			{
				UGameplayStatics::PlaySound2D(this, MeleeWeapon->SwingSounds[SwingSoundIndex]);
				++SwingSoundIndex;
			}
		}
	}
}


float AMain::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                        AActor* DamageCauser)
{
	Health -= DamageAmount;

	if (Health <= 0.0f)
	{
		Die();

		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);

			if (Enemy)
			{
				Enemy->SetHasValidTarget(false);
			}
		}
	}
	return DamageAmount;
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::Jump()
{
	if (MainPlayerController)
	{
		if (MainPlayerController->IsPauseMenuVisible())return;
	}

	if ((MovementStatus != EMovementStatus::EMS_Dead) && (!bBlocking) && (!bIsMeleeAttacking) && (!bIsDrawingArrow))
	{
		ResetIdleTimer();

		Super::Jump();
	}
}


void AMain::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}

		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);

	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();

		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}

		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();

	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);

		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	// Create an Instance of GameSave
	UGameSave* SaveGameInstance = Cast<UGameSave>(
		UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	// Save the Character's Stats
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.bInCombatMode = bInCombatMode;
	SaveGameInstance->CharacterStats.Coins = Coins;

	// Save the Character's Inventory.
	SaveGameInstance->Inventory.SetNum(InventoryComponent->NumberOfSlots);

	for (FSlotStructure Element : InventoryComponent->Inventory)
	{
		SaveGameInstance->Inventory.Add(Element);
	}

	// Save the name of the EquippedWeapon, if equipped.
	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.EquippedWeaponName = EquippedWeapon->GetItemStructure().ItemDisplayName;
	}

	// Save the name of the EquippedShield, if equipped.
	if (EquippedShield)
	{
		SaveGameInstance->CharacterStats.EquippedShieldName = EquippedShield->GetItemStructure().ItemDisplayName;
	}

	FString MapName = GetWorld()->GetMapName();

	//  Remove the "UEDPIE_0_" prefix from the name of the level
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;


	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName,
	                                 SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition)
{
	// Create an instance of GameSave
	UGameSave* LoadGameInstance = Cast<UGameSave>(
		UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	LoadGameInstance = Cast<UGameSave>(
		UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	if (LoadGameInstance)
	{
		Health = LoadGameInstance->CharacterStats.Health;
		MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
		Stamina = LoadGameInstance->CharacterStats.Stamina;
		MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
		bInCombatMode = LoadGameInstance->CharacterStats.bInCombatMode;
		Coins = LoadGameInstance->CharacterStats.Coins;


		/** Load Inventory */

		// Empty the Inventory
		InventoryComponent->Inventory.Empty();
		InventoryComponent->PrepareInventory();

		// Set size of the Inventory
		//InventoryComponent->NumberOfSlots = LoadGameInstance->InventoryComponent->NumberOfSlots;
		//InventoryComponent->PrepareInventory();

		// Add the Items from the Saved Inventory to the Character's Inventory IF not empty
		if (LoadGameInstance->Inventory.Num() > 0)
		{
			for (FSlotStructure Element : LoadGameInstance->Inventory)
			{
				InventoryComponent->AddToInventory(Element);
			}
		}

		// Load the Equipped Weapon if equipped
		if (LoadGameInstance->CharacterStats.EquippedWeaponName != TEXT(""))
		{
			for (FSlotStructure Element : InventoryComponent->Inventory)
			{
				// Check if any Item's name matches with the EquippedWeapon's name
				if (Element.ItemStructure.ItemDisplayName == LoadGameInstance->CharacterStats.EquippedWeaponName)
				{
					// Spawn and Equip the Weapon.
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Element.ItemStructure.ItemClass);
					WeaponToEquip->UseItem(this);
					break;
				}
			}
		}

			// else if no Weapon existed, destroy the Equipped Weapon, if any
		else
		{
			if (EquippedWeapon)
			{
				EquippedWeapon->Destroy();
				SetEquippedWeapon(nullptr);
			}
		}

		// Load the Equipped Shield if equipped
		if (LoadGameInstance->CharacterStats.EquippedShieldName != TEXT(""))
		{
			for (FSlotStructure Element : InventoryComponent->Inventory)
			{
				// Check if any Item's name matches with the EquippedShield's name
				if (Element.ItemStructure.ItemDisplayName == LoadGameInstance->CharacterStats.EquippedShieldName)
				{
					// Spawn and Equip the Shield.
					AShield* ShieldToEquip = GetWorld()->SpawnActor<AShield>(Element.ItemStructure.ItemClass);
					ShieldToEquip->UseItem(this);
					break;
				}
			}
		}
		/*	const FString WeaponName = LoadGameInstance->CharacterStats.EquippedWeaponName;
			
			if(WeaponName != TEXT(""))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>();
			}*/

		if (SetPosition)
		{
			SetActorLocation(LoadGameInstance->CharacterStats.Location);
			SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
		}

		SetMovementStatus(EMovementStatus::EMS_Normal);
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;

		// Get the name of the current map/level
		FString MapName = GetWorld()->GetMapName();
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		// If map name is not empty AND it is not the current map, change the level
		if ((LoadGameInstance->CharacterStats.LevelName != "")
			&& (LoadGameInstance->CharacterStats.LevelName != MapName))
		{
			SwitchLevel(*LoadGameInstance->CharacterStats.LevelName);
		}
	}
}

void AMain::LoadGameNoSwitch()
{
	UGameSave* LoadGameInstance = Cast<UGameSave>(
		UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	LoadGameInstance = Cast<UGameSave>(
		UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	bInCombatMode = LoadGameInstance->CharacterStats.bInCombatMode;
	Coins = LoadGameInstance->CharacterStats.Coins;

	/** Load Inventory */

	// Empty the Inventory
	InventoryComponent->Inventory.Empty();
	InventoryComponent->PrepareInventory();

	// Set size of the Inventory
	//InventoryComponent->NumberOfSlots = LoadGameInstance->InventoryComponent->NumberOfSlots;
	//InventoryComponent->PrepareInventory();

	// Add the Items from the Saved Inventory to the Character's Inventory IF not empty
	if (LoadGameInstance->Inventory.Num() > 0)
	{
		for (FSlotStructure Element : LoadGameInstance->Inventory)
		{
			InventoryComponent->AddToInventory(Element);
		}
	}

	// Load the Equipped Weapon if equipped
	if (LoadGameInstance->CharacterStats.EquippedWeaponName != TEXT(""))
	{
		for (FSlotStructure Element : InventoryComponent->Inventory)
		{
			// Check if any Item's name matches with the EquippedWeapon's name
			if (Element.ItemStructure.ItemDisplayName == LoadGameInstance->CharacterStats.EquippedWeaponName)
			{
				// Spawn and Equip the Weapon.
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Element.ItemStructure.ItemClass);
				WeaponToEquip->UseItem(this);
				break;
			}
		}
	}

	// Load the Equipped Shield if equipped
	if (LoadGameInstance->CharacterStats.EquippedShieldName != TEXT(""))
	{
		for (FSlotStructure Element : InventoryComponent->Inventory)
		{
			// Check if any Item's name matches with the EquippedShield's name
			if (Element.ItemStructure.ItemDisplayName == LoadGameInstance->CharacterStats.EquippedShieldName)
			{
				// Spawn and Equip the Shield.
				AShield* ShieldToEquip = GetWorld()->SpawnActor<AShield>(Element.ItemStructure.ItemClass);
				ShieldToEquip->UseItem(this);
				break;
			}
		}
	}

	// Get the name of the current map/level
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	// If map name is not empty AND it is the current map, set the position	of the Character.
	if ((LoadGameInstance->CharacterStats.LevelName != "")
		&& (LoadGameInstance->CharacterStats.LevelName == MapName))
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

bool AMain::CanCheckStaminaStatus()
{
	UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

	if (MainAnimInstance)
	{
		return (!GetMovementComponent()->IsFalling()
			&& !bBlocking
			&& !GetCharacterMovement()->IsCrouching()
			&& (bShiftKeyDown && bMovingForward && MainAnimInstance->GetSpeedForward() != 0.0f)
			&& !bIsDrawingArrow && !bIsBowAimed);
	}
	return false;
}

/**
 * Every time after an Idle Animation plays, the base Idle Animation should be played.
 * After the base Idle Animation is complete, one of the other Idle Animations will be randomly chosen.
 */
void AMain::IdleEnd(int32 PlayerStatusNo)
{
	// If the HUD is currently visible, remove it
	if (MainPlayerController->IsHUDVisible())
	{
		MainPlayerController->HideHUD();
	}

	// If Character was in Original Idle state, play an Idle Animation.
	if (IdleAnimSlot == 0)
	{
		// [0 -> NumberOfIdleAnims - 1]
		IdleAnimSlot = FMath::RandRange(1, NumberOfIdleAnims[PlayerStatusNo] - 1);
	}

		// else return back to the Original Idle state.
	else IdleAnimSlot = 0;
}

/**
	*First two Attack Sections are 1&2. Hence Ceiling of any of these divided by 2 = 1. HitSocketNames[1] gives "BodyFrontHitSocket".
	*Similarly for either of 3&4, Ceiling divided by 2 = 2. HitSocketNames[2] gives "HeadFrontHitSocket".
	*For rear hits, ceiling of either (1+4)&(2+4) i.e either 5&6 divided by 2 gives 3. HitSocketNames[3] gives "HeadFrontHitSocket"
	*Similarly HitSocketNames[4] gives "HeadRearHitSocket"
	*/
void AMain::SpawnHitParticles(AEnemy* DamageCauser)
{
	if (HitParticles)
	{
		UE_LOG(LogTemp, Warning, TEXT("BEFORE ENEMY ATTACK SECTION = %i"), DamageCauser->GetEnemyAttackSection());

		const int32 AttackSection =
			FMath::CeilToFloat(static_cast<float>(DamageCauser->GetEnemyAttackSection()) / 2.0f);

		UE_LOG(LogTemp, Warning, TEXT("After ENEMY ATTACK SECTION = %i"), AttackSection);

		UE_LOG(LogTemp, Warning, TEXT("Socket name = %s"), *(HitSocketNames[AttackSection - 1].ToString()));

		// Array indexes start at 0
		const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(HitSocketNames[AttackSection - 1]);

		if (TipSocket)
		{
			FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SocketLocation,
			                                         FRotator(0.0f), true);
		}
	}
}

void AMain::PlayHitSound()
{
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation(), FRotator(0, 0, 0));
}

void AMain::Footstep()
{
	// Line-trace will start from the Character's location to a point 500 units below it.
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 500.0f);

	// Hit result
	FHitResult OutHit;


	FCollisionQueryParams CollisionParams;

	// Ignore the Character during the Line-trace
	CollisionParams.AddIgnoredActor(this);

	// Make sure the Line-trace returns the Physical Material of the object it hits
	CollisionParams.bReturnPhysicalMaterial = true;

	// Line-trace from the Character to the ground
	bool Success = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams);

	// If the line-trace was successful
	if (Success)
	{
		EPhysicalMaterials PhysicalMaterial;

		// Find out the name of the Physical Material that the line-trace hit
		UObject* PhysMatObject = Cast<UObject>(OutHit.PhysMaterial.Get());

		if (PhysMatObject)
		{
			FName PhysMatObjectName = PhysMatObject->GetFName();

			// Assign the EPhysicalMaterials enum value accordingly
			if (PhysMatObjectName == "DefaultPhysicalMaterial" || PhysMatObjectName == "PhysicalMaterial_NormalLand")
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_NormalLand;
			}

			else if (PhysMatObjectName == "PhysicalMaterial_GrassyLand")
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_GrassyLand;
			}

			else if (PhysMatObjectName == "PhysicalMaterial_Stone")
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_Stone;
			}

			else if (PhysMatObjectName == "PhysicalMaterial_Wood")
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_Wood;
			}

			else if (PhysMatObjectName == "PhysicalMaterial_Water")
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_Water;
			}

			else
			{
				PhysicalMaterial = EPhysicalMaterials::EPM_NormalLand;
			}

			// Play the sound
			PlayFootstepSound(PhysicalMaterial, OutHit.ImpactPoint);

			UE_LOG(LogTemp, Warning, TEXT("Material Name: %s"), *OutHit.PhysMaterial.Get()->GetFName().ToString());
		}
	}
}

void AMain::PlayFootstepSound(EPhysicalMaterials& PhysicalMaterial, FVector& LocationToPlayAt)
{
	// Get the integer value of the enum variable
	const int32 PhysicalMaterialInt = static_cast<int32>(PhysicalMaterial);

	/**
	 * Each type of Physical material has indexes as below:
	 *
	 *	FYI: The '5' below is NumberOfFootstepSounds.
	 *
	 * {Index} - Name - [(5 * Index) -> ((5 * (Index + 1)) - 1)]
	 * {0} - Land - [0 -> 4]
	 * {1} - Stone - [5 -> 9]
	 * {2} - Wood - [10 -> 14]
	 * {3} - Water - [15 -> 19]
	 */

	int32 RandIndex = FMath::RandRange(NumberOfFootstepSounds * PhysicalMaterialInt,
	                                   (NumberOfFootstepSounds * (PhysicalMaterialInt + 1) - 1));

	if (MovementStatus == EMovementStatus::EMS_Normal)
	{
		// If the index decided is valid, play the Running footstep sound.
		if (RunningFootstepSounds.IsValidIndex(RandIndex))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), RunningFootstepSounds[RandIndex], LocationToPlayAt,
			                                      FRotator(0.0f, 0.0f, 0.0f));
		}
	}
	else if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		// If the index decided is valid, play the Sprinting footstep sound.
		if (SprintingFootstepSounds.IsValidIndex(RandIndex))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SprintingFootstepSounds[RandIndex], LocationToPlayAt,
			                                      FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}

//void AMain::UseItem(AItem* Item)
//{
//	// If not a Weapon and not a Shield
//	if (AWeapon* Weapon = Cast<AWeapon>(Item))
//	{
//		Weapon->Equip(this);
//	}
//	else if (AShield* Shield = Cast<AShield>(Item))
//	{
//		Shield->Equip(this);
//	}
//	else
//	{
//		Item->Use(this);
//		Item->OnUse(this);	// Blueprint Event
//	}
//}
