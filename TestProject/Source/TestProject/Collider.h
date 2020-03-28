// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

UCLASS()
class TESTPROJECT_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UColliderMovementComponent* OurMovementComponent;

	//getters

	virtual UPawnMovementComponent* GetMovementComponent() const override;
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() { return MeshComponent; }
	FORCEINLINE USphereComponent* GetSphereComponent() { return SphereComponent; }
	FORCEINLINE UCameraComponent* GetCameraComponent() { return CameraComponent; }
	FORCEINLINE USpringArmComponent* GetSpringArmComponent() { return SpringArmComponent; }



	//setters
	FORCEINLINE void SetMeshComponent(UStaticMeshComponent* MeshComp) { MeshComponent = MeshComp; }
	FORCEINLINE void SetSphereComponent(USphereComponent* SphereComp) { SphereComponent = SphereComp; }
	FORCEINLINE void SetCameraComponent(UCameraComponent* CameraComp) { CameraComponent = CameraComp; }
	FORCEINLINE void SetSpringArmComponent(USpringArmComponent* SpringArmComp) { SpringArmComponent = SpringArmComp; }



private:
	void MoveForward(float Value);
	void MoveRight(float Value);

};
