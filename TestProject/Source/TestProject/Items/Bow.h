// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "Items/Weapon.h"

#include "Bow.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API ABow : public AWeapon
{
	GENERATED_BODY()

public:

	ABow();

	UFUNCTION()
	void BowChargeTimelineProgress(float Value);

	UFUNCTION()
	void StartBowCharge();

	UFUNCTION()
	void EndBowCharge();

	UFUNCTION()
	void StopBowCharge();


protected:

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called every frame
	void Tick(float DeltaTime) override;

	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, Category = "ChargingTimeline")
	class UCurveFloat* CurveFloat;

	UPROPERTY(EditAnywhere, Category = "ChargingTimeline")
	float MaxCharge;

	// The Arrow the bow is going to shoot
	class AArrow* Arrow;


private:

	bool bIsBowCharging;
};
