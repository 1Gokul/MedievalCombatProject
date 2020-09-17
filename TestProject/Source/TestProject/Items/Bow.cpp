// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"

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
}

void ABow::EndBowCharge()
{
}

void ABow::StopBowCharge()
{
	bIsBowCharging = false;
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
		TimelineFinishedEvent.BindUFunction(this, FName("StopBowCharge"));
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
