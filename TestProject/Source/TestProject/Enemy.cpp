// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Shield.h"
#include "Kismet/KismetSystemLibrary.h"
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
AEnemy::AEnemy(){
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
	Damage = 10.0f;

	MinAttackTime = 0.50f;
	MaxAttackTime = 3.50f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.0f;

	CurrentAttackTipSocket = FName("");
	Attack1_TipSocket = FName("RightTipSocket");
	Attack2_TipSocket = FName("LeftTipSocket");
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
	LeftCombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftCombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftCombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightCombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	RightCombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightCombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightCombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightCombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);


}

// Called every frame
void AEnemy::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {
		AMain* Main = Cast<AMain>(OtherActor);
		
		if (Main) {
			MoveToTarget(Main);
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {

		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {

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

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) {

		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {

			bHasValidTarget = false;
			
			if (Main->CombatTarget == this) {
				Main->SetCombatTarget(nullptr);
			}
			Main->SetHasCombatTarget(false);

			Main->UpdateCombatTarget();

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);

			if (AIController) {

				AIController->StopMovement();
				CombatTarget = nullptr;

			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp) {

		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {

			/*if (Main->CombatTarget == this) {

				Main->SetCombatTarget(nullptr);
				Main->SetHasCombatTarget(false);

			}		*/	

			bOverlappingCombatSphere = false;

			MoveToTarget(Main);
			CombatTarget = nullptr;

			if (Main->CombatTarget == this) {
				Main->SetCombatTarget(nullptr);
				Main->bHasCombatTarget = false;
				Main->UpdateCombatTarget();
			}

			if (Main->MainPlayerController) {
				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);

				if (MainMesh) {
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

	if (AIController) {

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/*auto PathPoints = NavPath->GetPathPoints();

		for (auto Point : PathPoints) {
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.0f, 8, FLinearColor::Red, 10.0f, 1.0f);
		}*/
	}
}


void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) {

		AShield* Shield = Cast<AShield>(OtherActor);

		if (Shield) {

			//Just in case if the weapons come in contact with the player even after a block
			DeactivateCollisionLeft();
			DeactivateCollisionRight();

			//PLAY STAGGERED ANIMATION FOR ENEMY

			AMain* Char = Cast<AMain>(OtherActor);

			if (Char) {

				//PLAY STAGGERED ANIMATION FOR CHARACTER

				if (DamageTypeClass) {
					UGameplayStatics::ApplyDamage(Char, DamageIfBlocked, AIController, this, DamageTypeClass);
				}
			}

			if (Shield->BlockSound) {
				UGameplayStatics::PlaySound2D(this, Shield->BlockSound);
			}

			//Play spark animation
		}

		else {
			AMain* Char = Cast<AMain>(OtherActor);

			if (Char) {

				if (Char->HitParticles) {

					const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(CurrentAttackTipSocket);

					if (TipSocket) {
						FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Char->HitParticles, SocketLocation, FRotator(0.0f), true);

					}
				}
				if (Char->HitSound) {
					UGameplayStatics::PlaySound2D(this, Char->HitSound);
				}
				if (DamageTypeClass) {
					UGameplayStatics::ApplyDamage(Char, Damage, AIController, this, DamageTypeClass);
				}
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEnemy::ActivateCollisionLeft()
{
	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound) {
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemy::ActivateCollisionRight()
{
	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound) {
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

void AEnemy::Attack()
{
	if (Alive() && bHasValidTarget) {

		if (AIController) {
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking) {
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (AnimInstance) {
				//Randomly choose between the 2 attack animations
				int32 Section = FMath::RandRange(0, 1);

				switch (Section) {

				case 0:
					AnimInstance->Montage_Play(CombatMontage, 2.20f);
					AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
					CurrentAttackTipSocket = Attack1_TipSocket;
					break;

				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.80f);
					AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
					CurrentAttackTipSocket = Attack2_TipSocket;
					break;

				default:
					break;
				}
			}

		}
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere) {
		float AttackTime = FMath::FRandRange(MinAttackTime, MaxAttackTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.0f) {
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else {
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die(AActor* Causer)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance) {
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Dead;

	LeftCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent() ->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMain* Main = Cast<AMain>(Causer);

	if (Main) {

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


