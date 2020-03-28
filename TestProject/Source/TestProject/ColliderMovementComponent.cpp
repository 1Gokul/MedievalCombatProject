// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovementComponent.h"

void UColliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction) {

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	//Check to make sure everything is still valid and we are allowed to move

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) {
		return;
	}

	//Get and clear the vector in Collider
	FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MaxSpeed;

	if (!DesiredMovementThisFrame.IsNearlyZero()) {
		FHitResult Hit;

		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		// If we bump into something, slide along the side of it
		if (Hit.IsValidBlockingHit()) {
			SlideAlongSurface(DesiredMovementThisFrame, 1.0 - Hit.Time, Hit.Normal, Hit);
		}
	}

	
}