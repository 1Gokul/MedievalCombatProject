// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "Shield.h"


AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	bWeaponParticles = false;
	bShouldRotate = true;

	WeaponState = EWeaponState::EWS_Pickup;

	if (bIsTwoHanded)
	{
		Damage = 50.0f;
		BlockStaminaCost = 50.0f;
	}
	else
	{
		Damage = 25.0f;
	}

	HitSocketName = "ImpactSocket";
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);


	if (bIsTwoHanded)
	{
		SheathSocketName = FName("TopSpineSocket");
		HandSocketName = FName("RightHandSocket_TwoHanded");
	}
	else
	{
		SheathSocketName = FName("LeftThighSocket");
		HandSocketName = FName("RightHandSocket_OneHanded");
	}
}

bool AWeapon::UseItem(AMain* Main)
{
	// Call the base function
	Super::UseItem(Main);
	
	// If this is a Two-Handed Weapon, remove the equipped Shield, if any
	if (bIsTwoHanded)
	{
		if (Main->EquippedShield)
		{
			Main->EquippedShield->Destroy();
			Main->SetEquippedShield(nullptr);
		}
	}

	Main->SetEquippedWeapon(this);

	// Disable collision of the CollisionVolume
	CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

	SetInstigator(Main->GetController());
	SkeletalMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	SkeletalMesh->SetSimulatePhysics(false);

	bShouldRotate = false;

	if (!bWeaponParticles)
	{
		IdleParticlesComponent->Deactivate();
	}

	// Attach Weapon to Sheath Socket.
	Main->TimedSheathe();

	// Set bIsWeaponDrawn as false as the Weapon is sheathed.
	Main->bIsWeaponDrawn = false;

	// If in Combat Mode, draw the weapon.
	if (Main->bInCombatMode)
	{ 		
		Main->DrawWeapon();
	}

	return true;
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                             const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			Main->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                   const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);

		// If OtherActor is an Enemy
		if (Enemy)
		{
			//DeactivateCollision();

			// Deactivate CombatCollisions of the Enemy in case their attack was interrupted
			Enemy->DeactivateCollisionLeft();
			Enemy->DeactivateCollisionRight();

			Enemy->AttackEnd();

			//Play Enemy Impact Animation
			Enemy->Impact(MainAttackSection);

			if (Enemy->HitParticles)
			{
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");

				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);

					// Spawn a particle effect at WeaponSocket
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation,
					                                         FRotator(0.0f), true);
				}
			}
			if (Enemy->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			// Inflict damage on the Enemy
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
			}
		}
	}
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::Equip(AMain* Char)
{
	if (Char)
	{
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

		Char->bInCombatMode = true;

		SetInstigator(Char->GetController());
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName(HandSocketName);

		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Char->GetMesh());
			bShouldRotate = false;
			Char->SetEquippedWeapon(this);
			Char->SetActiveOverlappingItem(nullptr);
		}

		if (OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}

		if (!bWeaponParticles)
		{
			IdleParticlesComponent->Deactivate();
		}
	}
}

//void AWeapon::Sheath(AMain* Char)
//{
//	if (Char)
//	{
//		DeactivateCollision();
//
//		SetInstigator(nullptr);
//
//		const USkeletalMeshSocket* SheathSocket = Char->GetMesh()->GetSocketByName(SheathSocketName);
//
//		if (SheathSocket)
//		{
//			SheathSocket->AttachActor(this, Char->GetMesh());
//		}
//	}
//}
//
//void AWeapon::Unsheathe(AMain* Char)
//{
//	if (Char)
//	{
//		SetInstigator(nullptr);
//
//		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName(HandSocketName);
//
//		if (RightHandSocket)
//		{
//			RightHandSocket->AttachActor(this, Char->GetMesh());
//		}
//	}
//}
