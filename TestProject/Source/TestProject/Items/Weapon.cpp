// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Main.h"
#include "Shield.h"


AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	bWeaponParticles = false;
	bShouldRotate = true;

	WeaponState = EWeaponState::EWS_Pickup;

	SheathSocketName = FName(" ");
	HandSocketName = FName(" ");
	AttackAnimPrefix = " ";
	SheathAnimName = FName(" ");
	DrawAnimName = FName(" ");

	bIsTwoHanded = false;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

bool AWeapon::UseItem(AMain* Main)
{
	// Call the base function
	Super::UseItem(Main);

	// If this is a Two-Handed Weapon, remove the equipped Shield, if any
	if (bIsTwoHanded)
	{
		if (Main->GetEquippedShield())
		{
			Main->GetEquippedShield()->Destroy();
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
	TimedSheathe(Main);

	// Set bIsWeaponDrawn as false as the Weapon is sheathed.
	Main->SetbIsWeaponDrawn(false);

	// If in Combat Mode, draw the weapon.
	if (Main->GetInCombatMode())
	{
		Main->EnterCombatMode();
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

void AWeapon::Equip(AMain* Char)
{
	if (Char)
	{
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

		Char->SetInCombatMode(true);

		SetInstigator(Char->GetController());
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* HandSocket = Char->GetMesh()->GetSocketByName(HandSocketName);

		if (HandSocket)
		{
			HandSocket->AttachActor(this, Char->GetMesh());
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

void AWeapon::PlaySheathSound() const
{
	if (OnSheathSound)
	{
		UGameplayStatics::PlaySound2D(this, OnSheathSound);
	}
}

void AWeapon::PlayDrawSound() const
{
	if (OnEquipSound)
	{
		UGameplayStatics::PlaySound2D(this, OnEquipSound);
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

void AWeapon::TimedDraw(AMain* Main)
{
	SetInstigator(nullptr);

	const USkeletalMeshSocket* RightHandSocket = Main->GetMesh()->GetSocketByName(GetHandSocketName());

	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(this, Main->GetMesh());

		Main->SetIsWeaponDrawn(true);
	}
}

void AWeapon::TimedSheathe(AMain* Main)
{
	SetInstigator(nullptr);

	const USkeletalMeshSocket* SheathSocket = Main->GetMesh()->GetSocketByName(GetSheathSocketName());

	if (SheathSocket)
	{
		SheathSocket->AttachActor(this, Main->GetMesh());

		Main->SetIsWeaponDrawn(false);
	}
}
