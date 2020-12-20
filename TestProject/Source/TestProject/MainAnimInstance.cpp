// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Main.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}

void UMainAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.0f);
		MovementSpeed = LateralSpeed.Size();

		FVector VelocityTemp = Pawn->GetVelocity();
		VelocityTemp.Normalize();


		SpeedForward = FVector::DotProduct(Pawn->GetActorForwardVector(), Speed);
		SpeedRight = FVector::DotProduct(Pawn->GetActorRightVector(), Speed);


		if (Main)
		{
			// ControlRotation = Main->GetMesh()->GetComponentRotation();

			if (/*Main->GetIsDrawingArrow() || */MovementSpeed != 0.0f)
			{
				Pawn->bUseControllerRotationYaw = true;
			}
			else
			{
				Pawn->bUseControllerRotationYaw = false;
			}

			if (Main->GetIsDrawingArrow() || Main->GetIsBowAimed())
			{
				Pawn->bUseControllerRotationYaw = true;
			}
		}
		else
		{
			Main = Cast<AMain>(Pawn);
		}

		// UE_LOG(LogTemp, Warning, TEXT("Character Direction: (X = %f, Y = %f)"), SpeedForward, SpeedRight);

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();
	}
}
