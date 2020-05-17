// Fill out your copyright notice in the Description page of Project Settings.


#include "Shield.h"
#include "Components/StaticMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"

AShield::AShield() {

	BlockCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockCollision"));
	BlockCollision->SetupAttachment(GetRootComponent());

	ShieldState = EShieldState::ESS_Pickup;

	BlockStaminaCost = 20.0f;

}

void AShield::BeginPlay()
{
	Super::BeginPlay();

	BlockCollision->OnComponentBeginOverlap.AddDynamic(this, &AShield::CombatOnOverlapBegin);
	BlockCollision->OnComponentEndOverlap.AddDynamic(this, &AShield::CombatOnOverlapEnd);

	BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BlockCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BlockCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
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

void AShield::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AShield::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AShield::ActivateCollision()
{
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AShield::DeactivateCollision()
{
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
