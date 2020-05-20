// Fill out your copyright notice in the Description page of Project Settings.


#include "MainShield.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"

// Sets default values
AMainShield::AMainShield()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BlockCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockCollision"));
	RootComponent = BlockCollision;

	PickupVolume = CreateDefaultSubobject<USphereComponent>(TEXT("PickupVolume"));
	PickupVolume->SetupAttachment(GetRootComponent());

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
	
	ShieldState = EShieldState::ESS_Pickup;

	BlockStaminaCost = 20.0f;
}

// Called when the game starts or when spawned
void AMainShield::BeginPlay()
{
	Super::BeginPlay();

	BlockCollision->OnComponentBeginOverlap.AddDynamic(this, &AMainShield::CombatOnOverlapBegin);
	BlockCollision->OnComponentEndOverlap.AddDynamic(this, &AMainShield::CombatOnOverlapEnd);

	PickupVolume->OnComponentBeginOverlap.AddDynamic(this, &AMainShield::OnOverlapBegin);
	PickupVolume->OnComponentEndOverlap.AddDynamic(this, &AMainShield::OnOverlapEnd);

	BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BlockCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BlockCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
}

// Called every frame
void AMainShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMainShield::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((ShieldState == EShieldState::ESS_Pickup) && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main)
		{
			Main->SetActiveOverlappingShield(this);
		}
	}
}

void AMainShield::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
	if (OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);

		if (Main) {
			Main->SetActiveOverlappingShield(nullptr);
		}
	}
}

void AMainShield::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		/**
	Cast other actor and check if valid.
	if valid, play block reaction and sound.
	reduce stamina and health by a small amount
	*/

	//if (OtherActor) {
	//	AEnemy* Enemy = Cast<AEnemy>(OtherActor);

	//	if (Enemy && Enemy->bAttacking) {

	//		/**
	//			The animation going to be played below will interrupt
	//		*	the attacking animation of the Enemy. So as the DeactivateCollision
	//		*	AnimNotify will not be triggered, this will deactivate the 
	//		*	CombatCollision on both swords of the Enemy.
	//		*/
	//		

	//		Enemy->DeactivateCollisionLeft();
	//		Enemy->DeactivateCollisionRight();

	//		if (Enemy->CombatTarget) {
	//			Enemy->CombatTarget->Stamina -= BlockStaminaCost;
	//		}

	//		Enemy->AttackEnd();

	//	}
	//}
}

void AMainShield::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AMainShield::ActivateCollision()
{
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMainShield::DeactivateCollision()
{
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMainShield::Equip(AMain* Char)
{
	if (Char) {
		SetInstigator(Char->GetController());
		
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		StaticMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* LeftHandSocket = Char->GetMesh()->GetSocketByName("LeftHandSocket");


		if (LeftHandSocket) {
			LeftHandSocket->AttachActor(this, Char->GetMesh());
			Char->SetEquippedShield(this);
			Char->SetActiveOverlappingItem(nullptr);
		}

		if (OnEquipSound) {
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}
	}
}

