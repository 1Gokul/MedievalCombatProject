//  Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"


#include "Weapon.h"
#include "Shield.h"
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


//  Sets default values
AMain::AMain()
{
	//  Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm (pulls towards player)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());	// Attach to root component
	CameraBoom->TargetArmLength = 600.0f;	// Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true;		// Rotate arm based on controller

	// Set size of collision capsule
	GetCapsuleComponent()->SetCapsuleSize(36.0f, 92.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	// Attach camera to end of spring arm 
	FollowCamera->bUsePawnControlRotation = false;	// Will stay fixed to CameraBoom and will not rotate

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	Inventory->NumberOfSlots = 20;

	// Set out turn rates for input
	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;

	// To make sure the player doesn't rotate when the controller rotates and only the camera does.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character rotates to direction of input.
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

	OverlappingWeaponLocation = FVector(0.0, 0.0, 0.0);

	bShiftKeyDown = false;
	bLMBDown = false;
	bRMBDown = false;
	bFKeyDown = false;
	bCtrlDown = false;
	bTabDown = false;
	bEKeyDown = false;
	bInCombatMode = false;
	bAttacking = false;
	bBlocking = false;
	bCrouched = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
	bJumping = false;
	bInterpToEnemy = false;
	bIsWeaponEquipped = false;

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
	NumberOfIdleAnims.Add(6);
	NumberOfIdleAnims.Add(6);
	NumberOfIdleAnims.Add(6);
	NumberOfIdleAnims.Add(5);
	NumberOfIdleAnims.Add(4);

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
}

//  Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (bInCombatMode)
	{
		if (CurrentWeapon)
		{
			if (CurrentWeapon->bIsTwoHanded)SetPlayerStatus(EPlayerStatus::EPS_CombatArmed_2H);
			else SetPlayerStatus(EPlayerStatus::EPS_CombatArmed_1H);
		}
		else
		{
			if (EquippedShield)SetPlayerStatus(EPlayerStatus::EPS_ShieldUnarmed);
			else SetPlayerStatus(EPlayerStatus::EPS_UnarmedCombat);
		}
	}
	else
	{
		SetPlayerStatus(EPlayerStatus::EPS_UnarmedIdle);
	}

	//  Decrease in Stamina per second when sprinting.
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:

		if (CanCheckStaminaStatus())
		{	// or blocking

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

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		// Rotate to be directly facing the current Combat Target.
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();

		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}

	if (!GetWorldTimerManager().IsTimerActive(IdleAnimTimerHandle) && !bCrouched)
	{
		if (IdleAnimSlot == 0
			&& (Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance())->MovementSpeed == 0))
		{
			GetWorldTimerManager().SetTimer(IdleAnimTimerHandle, TimerDel, IdleTimeLimit, false);
		}
	}
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

	PlayerInputComponent->BindAction("CombatMode", IE_Pressed, this, &AMain::FKeyDown);
	PlayerInputComponent->BindAction("CombatMode", IE_Released, this, &AMain::FKeyUp);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMain::CtrlDown);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMain::CtrlUp);

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AMain::TabDown).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("Inventory", IE_Released, this, &AMain::TabUp).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::EKeyDown);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMain::EKeyUp);
}

bool AMain::bCanMove(float Value)
{
	if (MainPlayerController)
	{
		return (
			(Value != 0.0f)
			&& (!bAttacking)
			&& (MovementStatus != EMovementStatus::EMS_Dead)
			&& (!MainPlayerController->bPauseMenuVisible)
		);
	}
	return false;
}

void AMain::ResetIdleTimer()
{
	// Reset the Idle Anim Slot
	IdleAnimSlot = 0;

	// Pause IdleAnimTimerHandle if Active
	if(GetWorldTimerManager().IsTimerActive(IdleAnimTimerHandle))
	{
		GetWorldTimerManager().PauseTimer(IdleAnimTimerHandle);
	}
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;

	if (bCanMove(Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;

		ResetIdleTimer();
	}
}


void AMain::MoveRight(float Value)
{
	bMovingRight = false;

	if (bCanMove(Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;

		ResetIdleTimer();
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
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (MainPlayerController)
	{
		if (MainPlayerController->bPauseMenuVisible)return;
	}


	// If Player is overlapping with an Item, they can equip it 
	if (ActiveOverlappingItem)
	{
		// If the Item is a Weapon
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			if (CurrentWeapon)
			{
				if (bIsWeaponEquipped)
				{
					OverlappingWeaponLocation = Weapon->GetActorLocation();

					Weapon->Equip(this);
					SetActiveOverlappingItem(nullptr);
				}
			}
			else
			{
				OverlappingWeaponLocation = Weapon->GetActorLocation();

				Weapon->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}
		else
		{
			AShield* Shield = Cast<AShield>(ActiveOverlappingItem);
			if (Shield)
			{
				Shield->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}
	}

		/** else if Player already has a weapon equipped AND
		*	is not already blocking, perform an Attack.
		*/
	else if (!bBlocking)
	{
		UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());
		if (MainAnimInstance)
		{
			// If Player is not currently falling
			if (!MainAnimInstance->bIsInAir)
			{
				if (bInCombatMode)MeleeAttack();
				else DrawWeapon();
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
		if (MainPlayerController->bPauseMenuVisible)return;
	}

	/** else if Player already has a shield equipped AND
	*	is not already attacking, perform a Block.
	*/
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{

		ResetIdleTimer();
		
		// Blocking with a Weapon should only be allowed if the weapon is Two-Handed.
		if (CurrentWeapon && !EquippedShield)
		{
			if (CurrentWeapon->bIsTwoHanded)
			{
				UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());
				if (MainAnimInstance)
				{
					// If Player is not currently falling
					if (!MainAnimInstance->bIsInAir)
					{
						if (bInCombatMode)
						{
							if (!bBlocking)
							{
								bBlocking = true;
							}
						}
						else bInCombatMode = true;
					}
				}
			}
		}
		else
		{
			UMainAnimInstance* MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

			if (MainAnimInstance)
			{
				// If Player is not currently falling
				if (!MainAnimInstance->bIsInAir)
				{
					if (bInCombatMode)
					{
						if (!bBlocking)
						{
							bBlocking = true;
						}
					}
					else bInCombatMode = true;
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
		if(!MainPlayerController->bInventoryMenuVisible){
			MainPlayerController->TogglePauseMenu();
		}
	}
}

void AMain::FKeyUp()
{
	bFKeyDown = false;
}


void AMain::FKeyDown()
{
	bFKeyDown = true;

	ResetIdleTimer();

	/** If already in Combat Mode, switch back to Normal Mode and Sheathe the Weapon, if equipped. */
	if (bInCombatMode && !bBlocking)
	{
		SheatheWeapon();
	}

		/** If in Normal Mode, switch to Combat Mode and draw the Weapon, if it exists.*/
	else
	{
		DrawWeapon();
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

void AMain::SheatheWeapon()
{
	bInCombatMode = false;
	bAttacking = false;


	if (bIsWeaponEquipped && CurrentWeapon)
	{
		bIsWeaponEquipped = false;

		// Play the Sheath sound
		if (CurrentWeapon->OnSheathSound)
		{
			UGameplayStatics::PlaySound2D(this, CurrentWeapon->OnSheathSound);
		}

		// Play the Sheath Animation
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);

			if (CurrentWeapon->bIsTwoHanded)
				AnimInstance->Montage_JumpToSection(FName("SheatheWeapon_TwoHanded"), UpperBodyMontage);

			else AnimInstance->Montage_JumpToSection(FName("SheatheWeapon_OneHanded"), UpperBodyMontage);
		}
	}
}

void AMain::DrawWeapon()
{
	bInCombatMode = true;

	if (!bIsWeaponEquipped && CurrentWeapon)
	{
		bIsWeaponEquipped = true;

		// Play the Draw sound
		if (CurrentWeapon->OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, CurrentWeapon->OnEquipSound);
		}

		// Play the Draw Animation
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);

			if (CurrentWeapon->bIsTwoHanded)
				AnimInstance->Montage_JumpToSection(FName("DrawWeapon_TwoHanded"), UpperBodyMontage);
			else AnimInstance->Montage_JumpToSection(FName("DrawWeapon_OneHanded"), UpperBodyMontage);
		}
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

		GetCapsuleComponent()->SetCapsuleRadius(36.0);
	}
	else
	{
		bCrouched = true;

		PlayerCrouch();

		GetCapsuleComponent()->SetCapsuleRadius(60.0);
	}
}

void AMain::TabUp()
{
	bTabDown = false;
}

void AMain::TabDown()
{
	bTabDown = true;

	if(MainPlayerController)
	{
		// If a UI Widget (Other than the HUD) is not currently active, display the Inventory menu.
		if(!MainPlayerController->bPauseMenuVisible){

			if(MainPlayerController->bInventoryMenuVisible)
			{
				MainPlayerController->RemoveInventoryMenu();
			}
			else
			{
				 MainPlayerController->DisplayInventoryMenu(Inventory);
			}
			
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

	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors, ActorFilter);

	for (AActor* OverlappingActor : OverlappingActors)
	{		
		//  if the OverlappingActor implements UInteractInterface
		if(OverlappingActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
		{
			Cast<IInteractInterface>(OverlappingActor)->Interact();

			break;
		}
	}
}


void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}


void AMain::PlayMeleeAttack(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();


	// Attack Sections start from 1
	Section += 1;


	if (AnimInstance && CombatMontage)
	{
		if (CurrentWeapon)CurrentWeapon->MainAttackSection = Section;


		FString AttackName;

		// Append Section number

		// If Weapon attack
		if (bIsWeaponEquipped)
		{
			if (CurrentWeapon->bIsTwoHanded)AttackName.Append("TwoHandedAttack_");
			else AttackName.Append("OneHandedAttack_");

			if (!bCrouched)
			{
				// Get a random Final Combo Attack (Ranges from Attack_3 to Attack_5)
				if (Section == 3)Section += FMath::RandRange(0, 2);
			}
			else
			{	// Only one Section for crouched attacks
				Section = 1;
				AttackName.Append("Crouched_");
			}
		}
			// else if melee attack
		else
		{
			// If Player has a Shield equipped, play the even-numbered attacks only.
			if (PlayerStatus == EPlayerStatus::EPS_ShieldUnarmed)
			{
				// Override Section
				Section = FMath::RandRange(2, 4);
				if (Section % 2)Section -= 1;	// Set Section to play the 2nd Animation if RandRange returns 3.
			}

			AttackName.Append("MeleeAttack_");
		}


		/*	if (GetCharacterMovement()->IsCrouching())
			{
				AttackName.Append("Crouched_");
				Section = 1;
			}*/

		AttackName.AppendInt(Section);

		UE_LOG(LogTemp, Warning, TEXT("Attack = %s"), *AttackName);

		// Play Montage
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*AttackName, CombatMontage);

		/*	switch (Section) {
	
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.20f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;
	
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.80f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;
	
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.80f);
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage);
				break;
	
			default:
				break;
			} */
	}
}

void AMain::ResetMeleeAttackComboSection()
{
	AttackComboSection = 0;
}

void AMain::MeleeAttack()
{
	ResetIdleTimer();
	
	if (!bAttacking && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		bAttacking = true;

		SetInterpToEnemy(true);

		PlayMeleeAttack((AttackComboSection++) % 3);
	}
}

void AMain::MeleeAttackEnd()
{
	bAttacking = false;

	SetInterpToEnemy(false);

	// Reset Swing Sound index
	SwingSoundIndex = 0;

	// Reset Combo Attack Section if Player does not press LMB within AttackComboSectionResetTime.
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMain::ResetMeleeAttackComboSection,
	                                AttackComboSectionResetTime);
	// If Player is still holding LMBDown, attack again.
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
		FString HitName("Hit_");

		HitName.AppendInt(Section);

		// Play Montage
		AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*HitName, UpperBodyMontage);
	}
}


void AMain::BlockImpact(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && UpperBodyMontage)
	{
		FString ImpactName("Impact_");

		ImpactName.AppendInt(Section);

		// Play Montage
		AnimInstance->Montage_Play(UpperBodyMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(*ImpactName, UpperBodyMontage);
	}
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
	else if (bBlocking)
	{
		GetCharacterMovement()->MaxWalkSpeed = BlockingMaxWalkSpeed;
	}
	else if (bInCombatMode)
	{
		if (GetCharacterMovement()->IsCrouching())
			GetCharacterMovement()->MaxWalkSpeedCrouched =
				CrouchedCombatMaxWalkSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = CombatMaxWalkSpeed;
	}

	else
	{
		if (GetCharacterMovement()->IsCrouching())GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedMaxWalkSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::SetCurrentWeapon(AWeapon* WeaponToSet)
{
	if (CurrentWeapon)
	{
		// Swap weapons
		AWeapon* Temp = WeaponToSet;
		WeaponToSet = CurrentWeapon;
		CurrentWeapon = Temp;

		// Reset old weapon to default state
		WeaponToSet->SetWeaponState(EWeaponState::EWS_Pickup);
		WeaponToSet->bShouldRotate = true;
		WeaponToSet->SkeletalMesh->SetSimulatePhysics(true);

		// Detach from Character
		FDetachmentTransformRules DetachRules = FDetachmentTransformRules::KeepWorldTransform;
		WeaponToSet->DetachFromActor(DetachRules);

		// Set it at the location of the new weapon
		WeaponToSet->SetActorLocation(OverlappingWeaponLocation);
	}

	else CurrentWeapon = WeaponToSet;
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
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::PlaySwingSound()
{
	// Each weapon has specefic Swing Sounds depending on the current Attack number.
	if (SwingSoundIndex < CurrentWeapon->SwingSounds.Num())
	{
		if (CurrentWeapon->SwingSounds[SwingSoundIndex])
		{
			UGameplayStatics::PlaySound2D(this, CurrentWeapon->SwingSounds[SwingSoundIndex]);
			++SwingSoundIndex;
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
				Enemy->bHasValidTarget = false;
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
		if (MainPlayerController->bPauseMenuVisible)return;
	}

	if ((MovementStatus != EMovementStatus::EMS_Dead) && (!bBlocking) && (!bAttacking))
	{
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
	UGameSave* SaveGameInstance = Cast<UGameSave>(
		UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	if (CurrentWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = CurrentWeapon->Name;
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
		Coins = LoadGameInstance->CharacterStats.Coins;

		if (WeaponStorage)
		{
			// Create an Instance of WeaponStorage
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);

			if (Weapons)
			{
				FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

				if (WeaponName != TEXT(""))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}
			}
		}

		if (SetPosition)
		{
			SetActorLocation(LoadGameInstance->CharacterStats.Location);
			SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
		}

		SetMovementStatus(EMovementStatus::EMS_Normal);
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;


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
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		// Create an Instance of WeaponStorage
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);

		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (WeaponName != TEXT(""))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}


	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

bool AMain::CanCheckStaminaStatus()
{
	return ((!GetMovementComponent()->IsFalling() && !bBlocking && !GetCharacterMovement()->IsCrouching()) && (
		bShiftKeyDown && (bMovingForward ||
			bMovingRight)));
}

/**
 *Every time after an Idle Animation plays, the base Idle Animation should be played.
 * After the base Idle Animation is complete, one of the other Idle Animations will be randomly chosen.
 */
void AMain::IdleEnd(int32 PlayerStatusNo)
{
	if (IdleAnimSlot == 0)
	{
		IdleAnimSlot = FMath::RandRange(1, NumberOfIdleAnims[PlayerStatusNo] - 1);
	}
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
	UE_LOG(LogTemp, Warning, TEXT("BEFORE ENEMY ATTACK SECTION = %i"), DamageCauser->AttackSection);

	DamageCauser->AttackSection = FMath::CeilToFloat(static_cast<float>(DamageCauser->AttackSection) / 2.0f);

	UE_LOG(LogTemp, Warning, TEXT("Socket name = %s"), *(HitSocketNames[DamageCauser->AttackSection - 1].ToString()));

	// Array indexes start at 0
	const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(HitSocketNames[DamageCauser->AttackSection - 1]);

	if (TipSocket)
	{
		FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SocketLocation,
		                                         FRotator(0.0f), true);
	}
}

void AMain::UseItem(AItem* Item)
{
	// If not a Weapon and not a Shield
	if (AWeapon* Weapon = Cast<AWeapon>(Item))
	{
		Weapon->Equip(this);
	}
	else if (AShield* Shield = Cast<AShield>(Item))
	{
		Shield->Equip(this);
	}
	else
	{
		Item->Use(this);
		Item->OnUse(this);	// Blueprint Event
	}
}