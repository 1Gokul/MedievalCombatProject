// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "Weapon.h"
#include "Shield.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "GameSave.h"
#include "ItemStorage.h"
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


// Sets default values
AMain::AMain()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Spring Arm (pulls towards player)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());	//Attach to root component
	CameraBoom->TargetArmLength = 600.0f;	//Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true;		//Rotate arm based on controller

	//Set size of collision capsule
	GetCapsuleComponent()->SetCapsuleSize(36.0f, 92.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);	//Attach camera to end of spring arm 
	FollowCamera->bUsePawnControlRotation = false;	//Will stay fixed to CameraBoom and will not rotate

	//Set out turn rates for input
	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;

	//To make sure the player doesn't rotate when the controller rotates and only the camera does.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; //Character rotates to direction of input.
	GetCharacterMovement()->RotationRate = FRotator(0.0, 1000.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 650.0f;
	GetCharacterMovement()->AirControl = 0.20f;

	MaxHealth = 100.0f;
	Health = 65.0f;
	MaxStamina = 150.0f;
	Stamina = 120.0f;
	Coins = 0;

	RunningSpeed = 600.0f;
	SprintingSpeed = 850.0f;

	NormalWalkSpeed = 600.0f;
	BlockingWalkSpeed = 150.0f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bRMBDown = false;
	bAttacking = false;
	bBlocking = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
	bInterpToEnemy = false;

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.0f;
	MinSprintStamina = 50.0f;

	InterpSpeed = 15.0f;
	
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	//If the level is not the first level,
	if (MapName != "SunTemple") {

		//Load all items of player after level transition
		LoadGameNoSwitch();

		//Save the game.
		SaveGame();
	}

	//Return to game mode if game was loaded from the pause menu.
	if (MainPlayerController) {
		MainPlayerController->GameModeOnly();
	}

}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	// Decrease in Stamina per second when sprinting.
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus) {

	case EStaminaStatus::ESS_Normal:

		if (bShiftKeyDown) {

			if (Stamina - DeltaStamina <= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else {
				Stamina -= DeltaStamina;
			}

			if ((bMovingForward || bMovingRight) && (!bBlocking)) {
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else {
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else {
			if (Stamina + DeltaStamina >= MaxStamina) {
				Stamina = MaxStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:

		if (bShiftKeyDown) {
			if (Stamina - DeltaStamina <= 0.0f) {
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else {
				Stamina -= DeltaStamina;
				if ((bMovingForward || bMovingRight) && (!bBlocking)) {
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else {
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}
		else {
			if (Stamina + DeltaStamina >= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:

		if (bShiftKeyDown) {
			Stamina = 0.0f;
		}
		else {
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:

		if (Stamina + DeltaStamina >= MinSprintStamina) {
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else {
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	default:
		break;
	}

	if (bInterpToEnemy && CombatTarget) {
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		//Rotate to be directly facing the current Combat Target.
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();

		if (MainPlayerController) {
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMB", IE_Released, this, &AMain::RMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);


	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

bool AMain::bCanMove(float Value)
{
	if (MainPlayerController) {

		return(
			(Value != 0.0f)
			&& (!bAttacking)			
			&& (MovementStatus != EMovementStatus::EMS_Dead)
			&& (!MainPlayerController->bPauseMenuVisible)
			);

	}

	else {
		return false;
	}
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;

	if (bCanMove(Value)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}

void AMain::Turn(float Value)
{
	if (bCanMove(Value)) {
		AddControllerYawInput(Value);
	}
}
void AMain::LookUp(float Value)
{
	if (bCanMove(Value)) {
		AddControllerPitchInput(Value);
	}
}

void AMain::MoveRight(float Value)
{
	bMovingRight = false;

	if (bCanMove(Value)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
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

	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible)return;
	}

	//If Player is overlapping with an Item, they can equip it 
	if (ActiveOverlappingItem) {

		//If the Item is a Weapon
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon) {
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
		else{
			AShield* Shield = Cast<AShield>(ActiveOverlappingItem);
			if(Shield)
			{
				Shield->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}

	}

	/** else if Player already has a weapon equipped AND
	*	is not already blocking, perform an Attack.
	*/
	else if (EquippedWeapon && !bBlocking) {
		Attack();
	}
}

void AMain::RMBUp()
{
	bBlocking = false;
	bRMBDown = false;

	 //GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	//EquippedShield->DeactivateCollision();
}

void AMain::RMBDown()
{
	bRMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible)return;
	}

	/** else if Player already has a shield equipped AND
	*	is not already attacking, perform a Block.
	*/
	if (EquippedShield && !bAttacking)
	{
		if (MovementStatus != EMovementStatus::EMS_Dead)
		{
			//Activate collision so that the MainShield can detect Overlaps.
			//EquippedShield->ActivateCollision();
			//
			//Lower speed of walking
			//GetCharacterMovement()->MaxWalkSpeed = BlockingWalkSpeed;
			
			if (!bBlocking)bBlocking = true;
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

	
	if (MainPlayerController) {
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::Attack()
{
	if (!bAttacking && (MovementStatus != EMovementStatus::EMS_Dead)) {
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && CombatMontage) {

			//Randomly choose between the 2 attack animations
			int32 Section = FMath::RandRange(0, 1);

			switch (Section) {

			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.20f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.80f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;

			default:
				break;
			}

		}

	}

}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	//If Player is still holding LMBDown, attack again.
	if (bLMBDown) {
		Attack();
	}
}

//NOT BEING USED
void AMain::Block()
{
	//if (MovementStatus != EMovementStatus::EMS_Dead) {

	//	//Activate collision so that the MainShield can detect Overlaps.
	//	EquippedShield->ActivateCollision();



	//	if(!bBlocking)bBlocking = true;
		//Combat Montage method
		
		////if Player was not already blocking
		//if (!bBlocking) {
		//	bBlocking = true;

		//	//Play the "Going To Block" Animation
		//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		//	if (AnimInstance && CombatMontage) {

		//		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		//		AnimInstance->Montage_JumpToSection(FName("BlockStart"), CombatMontage);

		//	}
		//}
		////If already blocking,
		//else{

		//	//Play the "Blocking Idle" animation
		//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		//	if (AnimInstance && CombatMontage) {
		//			AnimInstance->Montage_Play(CombatMontage, 3.0f);
		//			AnimInstance->Montage_JumpToSection(FName("BlockIdle"), CombatMontage);
		//	}
		//}
	//}
}

//NOT BEING USED
void AMain::BlockEnd()
{
	//if (bRMBDown) {
	//	Block();
	//}

	//else{
	//	//Play the animation to go from blocking stance to normal
	//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//	if (AnimInstance && CombatMontage) {

	//		AnimInstance->Montage_Play(CombatMontage, 3.0f);
	//		AnimInstance->Montage_JumpToSection(FName("BlockEnd"), CombatMontage);

	//	}

	//	bBlocking = false;
	//}
}

void AMain::Impact(int32 Section)
{
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage) {

		switch (Section) {

		case 0:
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("HitFromBehind"), CombatMontage);
			break;

		//case 1:
		//	AnimInstance->Montage_Play(CombatMontage, 1.0f);
		//	AnimInstance->Montage_JumpToSection(FName("Impact_2"), CombatMontage);
		//	break;

		//case 2:
		//	AnimInstance->Montage_Play(CombatMontage, 1.0f);
		//	AnimInstance->Montage_JumpToSection(FName("Impact_3"), CombatMontage);
		//	break;

		default:
			break;
		}

	}
}


void AMain::BlockImpact(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage) {

		switch (Section) {

		case 0:
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Impact_1"), CombatMontage);
			break;

		case 1:
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Impact_2"), CombatMontage);
			break;

		case 2:
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Impact_3"), CombatMontage);
			break;

		default:
			break;
		}

	}
}


void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
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

	if (Health >= MaxHealth) {
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

	if (MovementStatus == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else if(bBlocking)
	{
		GetCharacterMovement()->MaxWalkSpeed = BlockingWalkSpeed;
	}
	
	else{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeaponToSet;
}

void AMain::SetEquippedShield(AShield* ShieldToSet)
{
	if (EquippedShield) {
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
	if (EquippedWeapon->SwingSound) {
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMain::SetInterpToEnemy(bool Interp) {

	bInterpToEnemy = Interp;
}

float AMain::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;

	if (Health <= 0.0f) {
		Die();

		if (DamageCauser) {
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);

			if (Enemy) {
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
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible)return;
	}

	if ((MovementStatus != EMovementStatus::EMS_Dead) && (!bBlocking)) {
		Super::Jump();
	}
}

void AMain::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0) {

		if (MainPlayerController) {
			MainPlayerController->RemoveEnemyHealthBar();
		}

		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);

	if (ClosestEnemy) {
		FVector Location = GetActorLocation();

		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors) {
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy) {
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance) {
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}

		if (MainPlayerController) {
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();

	if (World) {
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);

		if (CurrentLevelName != LevelName) {
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	UGameSave* SaveGameInstance = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	if (EquippedWeapon) {
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	FString MapName = GetWorld()->GetMapName();

	// Remove the "UEDPIE_0_" prefix from the name of the level
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;



	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition)
{
	UGameSave* LoadGameInstance = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	LoadGameInstance = Cast<UGameSave>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	if (LoadGameInstance) {

		Health = LoadGameInstance->CharacterStats.Health;
		MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
		Stamina = LoadGameInstance->CharacterStats.Stamina;
		MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
		Coins = LoadGameInstance->CharacterStats.Coins;

		if (WeaponStorage) {

			//Create an Instance of WeaponStorage
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);

			if (Weapons) {
				FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

				if (WeaponName != TEXT("")) {
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}

			}
		}

		if (SetPosition) {
			SetActorLocation(LoadGameInstance->CharacterStats.Location);
			SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
		}

		SetMovementStatus(EMovementStatus::EMS_Normal);
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;


		FString MapName = GetWorld()->GetMapName();
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		//If map name is not empty AND it is not the current map, change the level
		if ((LoadGameInstance->CharacterStats.LevelName != "")
				&& (LoadGameInstance->CharacterStats.LevelName != MapName)){

			SwitchLevel(*LoadGameInstance->CharacterStats.LevelName);
		}
	}

}

void AMain::LoadGameNoSwitch()
{
	UGameSave* LoadGameInstance = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));

	LoadGameInstance = Cast<UGameSave>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) {

		//Create an Instance of WeaponStorage
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);

		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (WeaponName != TEXT("")) {

				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);

			}

		}
	}


	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}
