// Fill out your copyright notice in the Description page of Project Settings.


#include "Shield.h"
#include "Components/StaticMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Enemy.h"

AShield::AShield() {


	ShieldState = EShieldState::ESS_Pickup;

	BlockStaminaCost = 30.0f;

	HitSocketName = "ImpactSocket";

}

void AShield::BeginPlay()
{
	Super::BeginPlay();

}

void AShield::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if ((ShieldState == EShieldState::ESS_Pickup) && OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AShield::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {
			Main->SetActiveOverlappingItem(nullptr);
		}
	}
}




void AShield::Equip(AMain* Char)
{
	if (Char) {
		SetInstigator(Char->GetController());
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		StaticMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* LeftHandSocket = Char->GetMesh()->GetSocketByName("LeftHandSocket");


		if (LeftHandSocket) {
			LeftHandSocket->AttachActor(this, Char->GetMesh());
			bShouldRotate = false;
			Char->SetEquippedShield(this);
			Char->SetActiveOverlappingItem(nullptr);
		}

		if (OnEquipSound) {
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}
	}
}
