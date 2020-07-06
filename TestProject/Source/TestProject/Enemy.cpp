// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Items/Weapon.h"
#include "Items/Shield.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(1000.0f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.0f);

	LeftCombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftCombatCollision"));
	LeftCombatCollision->SetupAttachment(GetMesh(), FName("LeftEnemySocket"));

	RightCombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightCombatCollision"));
	RightCombatCollision->SetupAttachment(GetMesh(), FName("RightEnemySocket"));

	bOverlappingCombatSphere = false;
	bAttacking = false;
	bHasValidTarget = false;

	Health = 75.0f;
	MaxHealth = 100.0f;
	Damage = 15.0f;
	DamageIfBlocked = 10.0f;

	MinAttackTime = 0.50f;
	MaxAttackTime = 3.50f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.0f;

	AttackSection = -1;

	bInterpToEnemy = false;
	InterpSpeed = 15.0f;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	LeftCombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	LeftCombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftCombatCollision->SetCollisionObjectType(ECC_WorldDynamic);
	LeftCombatCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftCombatCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	RightCombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	RightCombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightCombatCollision->SetCollisionObjectType(ECC_WorldDynamic);
	RightCombatCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightCombatCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		//Rotate to be directly facing the current Combat Target.
		SetActorRotation(InterpRotation);
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			MoveToTarget(Main);
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			bHasValidTarget = true;
			Main->SetCombatTarget(this);
			Main->SetHasCombatTarget(true);

			Main->UpdateCombatTarget();

			CombatTarget = Main;
			bOverlappingCombatSphere = true;

			float AttackTime = FMath::FRandRange(MinAttackTime, MaxAttackTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			bHasValidTarget = false;

			if (Main->CombatTarget == this)
			{
				Main->SetCombatTarget(nullptr);
			}
			Main->SetHasCombatTarget(false);

			Main->UpdateCombatTarget();

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);

			if (AIController)
			{
				AIController->StopMovement();
				CombatTarget = nullptr;
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			/*if (Main->CombatTarget == this) {

				Main->SetCombatTarget(nullptr);
				Main->SetHasCombatTarget(false);

			}		*/

			bOverlappingCombatSphere = false;

			MoveToTarget(Main);
			CombatTarget = nullptr;

			if (Main->CombatTarget == this)
			{
				Main->SetCombatTarget(nullptr);
				Main->bHasCombatTarget = false;
				Main->UpdateCombatTarget();
			}

			if (Main->MainPlayerController)
			{
				USkeletalMeshComponent* CharMesh = Cast<USkeletalMeshComponent>(OtherComp);

				if (CharMesh)
				{
					Main->MainPlayerController->RemoveEnemyHealthBar();
				}
			}


			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

void AEnemy::MoveToTarget(AMain* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(50.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/*auto PathPoints = NavPath->GetPathPoints();

		for (auto Point : PathPoints) {
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.0f, 8, FLinearColor::Red, 10.0f, 1.0f);
		}*/
	}
}


void AEnemy::InflictDamageOnMain(AMain* Main, bool bHitFromBehind)
{
	//If Character was hit from behind
	if (bHitFromBehind)
	{
		Main->Impact(1);

		/**
		 * First 2 Attack Sockets in Main's SocketNames are the names of the Sockets that will be used
		 * if the Player gets hit by the Enemy facing them. By adding 4, we get the Sockets that will be used if
		 * the Player gets hit by the Enemy facing their back.	See below in if(Main->HitParticles) to understand
		 */
		AttackSection += 4;
	}

		//If Character was hit while facing the attacking Enemy
	else
	{
		//int32 Section = FMath::RandRange(1, 3);
		Main->Impact(AttackSection + 1);
	}

	if (Main->HitParticles)
	{
		Main->SpawnHitParticles(this);
	}
	if (Main->HitSound)
	{
		UGameplayStatics::PlaySound2D(this, Main->HitSound);
	}
	if (DamageTypeClass)
	{
		UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
	}

	//In case the Player's attack was interrupted
	Main->bAttacking = false;

	Main->bInterpToEnemy = false;
}

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			//Check if the player is facing the Enemy
			FVector MainLocation = Main->GetActorLocation();
			FVector EnemyLocation = GetActorLocation();

			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(MainLocation, EnemyLocation);

			FRotator MainRotation = Main->GetActorRotation();

			FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(MainRotation, LookAtRotation);

			//Acceptable range of rotation = -135 - 135 degrees
			bool bAngleCheck1 = UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -180, -60);
			bool bAngleCheck2 = UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, 60, 180);

			if (Main->bBlocking)
			{
				//If facing the Enemy
				if (!(bAngleCheck1 || bAngleCheck2))
				{
					//If Player is blocking with shield and has enough stamina to successfully block an attack  
					if (Main->EquippedShield && Main->Stamina - Main->EquippedShield->BlockStaminaCost >= 0)
					{
						Main->Stamina -= Main->EquippedShield->BlockStaminaCost;

						int32 Section = FMath::RandRange(1, 3);

						//Character Shield Block Impact Animation
						Main->BlockImpact(Section);

						if (Main->EquippedShield->BlockSound)
						{
							UGameplayStatics::PlaySound2D(this, Main->EquippedShield->BlockSound);
						}

						if (Main->EquippedShield->HitParticles)
						{
							FVector SocketLocation = Main->EquippedShield->StaticMesh->GetSocketLocation(
								Main->EquippedShield->HitSocketName);
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->EquippedShield->HitParticles,
							                                         SocketLocation, FRotator(0.0f), true);
						}
					}

						//If Player is blocking with weapon and has enough stamina to successfully block an attack
					else if (Main->bIsWeaponDrawn && Main->Stamina - Main->CurrentWeapon->BlockStaminaCost >= 0)
					{
						//Weapons don't block attacks completely
						if (DamageTypeClass)
						{
							UGameplayStatics::ApplyDamage(Main, DamageIfBlocked, AIController, this,
							                              DamageTypeClass);
						}

						Main->Stamina -= Main->CurrentWeapon->BlockStaminaCost;

						int32 Section = FMath::RandRange(4, 6);

						//Character Weapon Block Impact Animation
						Main->BlockImpact(Section);  // +3 because first 3 attack sections are for shield blocking

						if (Main->CurrentWeapon->BlockSound)
						{
							UGameplayStatics::PlaySound2D(this, Main->CurrentWeapon->BlockSound);
						}

						if (Main->CurrentWeapon->HitParticles)
						{
							const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(
								Main->CurrentWeapon->HitSocketName);

							if (TipSocket)
							{
								FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
								UGameplayStatics::SpawnEmitterAtLocation(
									GetWorld(), Main->CurrentWeapon->HitParticles, SocketLocation,
									FRotator(0.0f), true);
							}
						}
					}

						//If Player does not have enough stamina to successfully block an attack
					else
					{
						InflictDamageOnMain(Main, false);
					}
				}
				else
				{
					//If not facing the Enemy
					InflictDamageOnMain(Main, true);
				}
			}

			else
			{	//If not blocking
				InflictDamageOnMain(Main, (bAngleCheck1 || bAngleCheck2));
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEnemy::ActivateCollisionLeft()
{
	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemy::ActivateCollisionRight()
{
	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemy::DeactivateCollisionLeft()
{
	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DeactivateCollisionRight()
{
	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FRotator AEnemy::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.0f, LookAtRotation.Yaw, 0.0f);
	return LookAtRotationYaw;
}

void AEnemy::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

void AEnemy::Attack()
{
	if (Alive() && bHasValidTarget)
	{
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking)
		{
			SetInterpToEnemy(true);

			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (AnimInstance)
			{
				//Randomly choose between the 4 attack animations
				AttackSection = FMath::RandRange(1, 4);

				FString AttackName("Attack_");
				AttackName.AppendInt(AttackSection);

				AnimInstance->Montage_Play(CombatMontage, 1.0f);
				AnimInstance->Montage_JumpToSection(FName(*AttackName), CombatMontage);

				/*	switch (AttackSection)
					{
					case 0:
						AnimInstance->Montage_Play(CombatMontage, 1.0f);
						AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
						
						break;
	
					case 1:
						AnimInstance->Montage_Play(CombatMontage, 1.0f);
						AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
						break;
	
					case 2:
						AnimInstance->Montage_Play(CombatMontage, 1.0f);
						AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage);
						CurrentAttackTipSocket = Attack2_TipSocket;
						break;
	
					default:
						break;
					}*/
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(MinAttackTime, MaxAttackTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}


float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                         AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.0f)
	{
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die(AActor* Causer)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Dead;

	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMain* Main = Cast<AMain>(Causer);

	if (Main)
	{
		Main->UpdateCombatTarget();
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return (GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead);
}

void AEnemy::Disappear()
{
	Destroy();
}

void AEnemy::Impact(int32 Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage)
	{
		switch (Section)
		{
		case 0:
			AnimInstance->Montage_Play(CombatMontage, 1.5f);
			AnimInstance->Montage_JumpToSection(FName("HitLeft"), CombatMontage);
			break;

		case 1:
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("HitRight"), CombatMontage);
			break;

		case 2:
			AnimInstance->Montage_Play(CombatMontage, 1.5f);
			AnimInstance->Montage_JumpToSection(FName("HitFront"), CombatMontage);
			break;

		case 3:
			AnimInstance->Montage_Play(CombatMontage, 1.5f);
			AnimInstance->Montage_JumpToSection(FName("HitRear"), CombatMontage);
			break;

		default:
			break;
		}
	}
}
