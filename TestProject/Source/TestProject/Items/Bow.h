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

	// Gets the Character ready to shoot; increases the firing power/charge as the RMB is held down.
	UFUNCTION()
	void StartBowCharge();

	// Returns the Character back to a normal state from the Ready state. Reduce the firing power/charge back to zero.
	UFUNCTION()
	void EndBowCharge();

	// Stops charging; called after the bow has finished charging.
	UFUNCTION()
	void StopBowCharge();

	// Called in BP so that the arrow gets attached to the Character's hand at the right moment in the "Arrow Draw" animation.
	UFUNCTION(BlueprintCallable)
	void TimedArrowDraw(AMain* Main);


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
