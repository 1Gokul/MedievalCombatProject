// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "Arrow.h"
#include "Main.h"
#include "Kismet/GameplayStatics.h"

ABow::ABow()
{
	// Allow the Bow to Tick every frame.
	PrimaryActorTick.bCanEverTick = true;

	WeaponType = EWeaponType::EWT_Bow;

	bIsBowCharging = false;
}

void ABow::BowChargeTimelineProgress(float Value)
{
	float NewChargeAmount = FMath::Lerp(0.0f, MaxCharge, Value);
}

void ABow::StartBowCharge()
{
	if (Arrow)
	{
		CurveTimeline.Play();
	}
}

void ABow::EndBowCharge()
{
	if (Arrow)
	{
		CurveTimeline.Reverse();
	}
}

void ABow::StopBowCharge()
{
	CurveTimeline.Stop();

	// Set the Charge value to the starting value
	CurveTimeline.SetNewTime(0.0f);

	bIsBowCharging = false;
}

void ABow::TimedArrowDraw(AMain* Main)
{
	Arrow = Cast<AArrow>(GetWorld()->SpawnActor(AArrow::StaticClass()));


	AttachToComponent(Main->GetMesh(),
	                  FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
	                                            EAttachmentRule::KeepWorld, true),
	                  FName("ArrowAttachSocket"));
}

void ABow::BeginPlay()
{
	Super::BeginPlay();

	SheathSocketName = "SpineSheathSocket_Bow";
	HandSocketName = "LeftHandSocket_Bow";
	AttackAnimPrefix = "FireBow";
	SheathAnimName = FName("SheatheWeapon_Bow");
	DrawAnimName = FName("DrawWeapon_Bow");

	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		FOnTimelineEventStatic TimelineFinishedEvent;

		TimelineProgress.BindUFunction(this, FName("BowChargeTimelineProgress"));
		// TimelineFinishedEvent.BindUFunction(this, FName("StopBowCharge"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetTimelineLength(1.50f);
	}
}

void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If the bow is currently pulled and is charging,
	if (bIsBowCharging)
	{
		CurveTimeline.TickTimeline(DeltaTime);
	}
}
