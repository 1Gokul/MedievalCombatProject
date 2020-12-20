// Fill out your copyright notice in the Description page of Project Settings.


#include "Shield.h"
#include "Components/StaticMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Enemy.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"

AShield::AShield()
{
	ShieldState = EShieldState::ESS_Pickup;

	BlockStaminaCost = 30.0f;

	HitSocketName = "ImpactSocket";
}

void AShield::BeginPlay()
{
	Super::BeginPlay();
}

bool AShield::UseItem(AMain* Main)
{
	// Call the base function
	Super::UseItem(Main);

	if (Main->GetEquippedWeapon())
	{
		if (Main->GetEquippedWeapon()->IsTwoHanded())
		{
			Main->GetEquippedWeapon()->Destroy();
			Main->SetEquippedWeapon(nullptr);
			Main->SetIsWeaponDrawn(false);
		}
	}
	CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

	SetInstigator(Main->GetController());
	StaticMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	StaticMesh->SetSimulatePhysics(false);

	const USkeletalMeshSocket* LeftHandSocket = Main->GetMesh()->GetSocketByName("LeftHandSocket_Shield");


	if (LeftHandSocket)
	{
		LeftHandSocket->AttachActor(this, Main->GetMesh());
		bShouldRotate = false;
		Main->SetEquippedShield(this);
		Main->SetActiveOverlappingItem(nullptr);
	}

	return true;
}


void AShield::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                             const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if ((ShieldState == EShieldState::ESS_Pickup) && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AShield::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
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


void AShield::Equip(AMain* Char)
{
	if (Char)
	{
		Char->SetInCombatMode(true);

		SetInstigator(Char->GetController());
		StaticMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		StaticMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* LeftHandSocket = Char->GetMesh()->GetSocketByName("LeftHandSocket_Shield");


		if (LeftHandSocket)
		{
			LeftHandSocket->AttachActor(this, Char->GetMesh());
			bShouldRotate = false;
			Char->SetEquippedShield(this);
			Char->SetActiveOverlappingItem(nullptr);
		}

		if (OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}
	}
}

void AShield::PlayBlockSound()
{
	if (BlockSound)
	{
		UGameplayStatics::PlaySound2D(this, BlockSound);
	}
}

void AShield::EmitHitParticles()
{
	FVector SocketLocation = StaticMesh->GetSocketLocation(HitSocketName);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SocketLocation, FRotator(0.0f), true);
}
