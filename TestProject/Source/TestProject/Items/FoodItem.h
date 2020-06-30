// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API AFoodItem : public AItem
{
	GENERATED_BODY()

public:

	/** The amount by which the Player's health  will increase when this Food Item is consumed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float HealthToHeal;

protected:


public:
	bool UseItem(AMain* Main) override;
};
