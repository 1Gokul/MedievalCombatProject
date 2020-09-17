// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API UMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	void NativeInitializeAnimation() override;

	FORCEINLINE float GetMovementSpeed() const { return MovementSpeed; }

	FORCEINLINE float GetSpeedForward() const { return SpeedForward; }

	FORCEINLINE float GetSpeedRight() const { return SpeedRight; }

	FORCEINLINE bool IsInAir() const { return bIsInAir; }


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float SpeedForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float SpeedRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AMain* Main;
};
