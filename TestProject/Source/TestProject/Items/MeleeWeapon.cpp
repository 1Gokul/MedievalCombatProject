// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"

#include "Enemy.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AMeleeWeapon::AMeleeWeapon()
{
	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	HitSocketName = "ImpactSocket";
}

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AMeleeWeapon::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	if (WeaponType == EWeaponType::EWT_TwoHandedMelee)
	{
		Damage = 50.0f;
		BlockStaminaCost = 50.0f;
		AttackAnimPrefix = "TwoHandedMeleeAttack_";
		SheathSocketName = "SpineSheathSocket_TwoHandedMelee";
		HandSocketName = "RightHandSocket_TwoHandedMelee";
		SheathAnimName = FName("SheatheWeapon_TwoHandedMelee");
		DrawAnimName = FName("DrawWeapon_TwoHandedMelee");
	}
	else
	{
		Damage = 25.0f;
		AttackAnimPrefix = "OneHandedMeleeAttack_";
		SheathSocketName = "LeftThighSheathSocket_OneHandedMelee";
		HandSocketName = "RightHandSocket_OneHandedMelee";
		SheathAnimName = FName("SheatheWeapon_OneHandedMelee");
		DrawAnimName = FName("DrawWeapon_OneHandedMelee");
	}
}

void AMeleeWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void AMeleeWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AMeleeWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMeleeWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMeleeWeapon::PlayBlockSound()
{
	if (BlockSound)
	{
		UGameplayStatics::PlaySound2D(this, BlockSound);
	}
}

void AMeleeWeapon::EmitHitParticles()
{
	if (HitParticles)
	{
		const USkeletalMeshSocket* TipSocket = SkeletalMesh->GetSocketByName(HitSocketName);

		if (TipSocket)
		{
			FVector SocketLocation = TipSocket->GetSocketLocation(SkeletalMesh);
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(), HitParticles, SocketLocation,
				FRotator(0.0f), true);
		}
	}
}
