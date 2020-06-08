// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingPlatform.generated.h"

UCLASS()
class TESTPROJECT_API AFloatingPlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFloatingPlatform();

	/** Mesh for the platform */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Platform)
	class UStaticMeshComponent* Mesh;

	/** Initial position of the platform*/
	UPROPERTY(EditAnywhere)
	FVector StartPoint;

	/** Final position of the platform*/
	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = "true"))
	FVector EndPoint;

	/** Interpolation speed at which the platform will move*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Platform)
	float InterpSpeed;

	/** Interpolation time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Platform)
	float InterpTime;

	FTimerHandle InterpTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Platform)
	bool bIsInterping;

	float Distance;
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

public:
	// Called every frame
	void Tick(float DeltaTime) override;

	void ToggleInterping();

	void SwapVectors(FVector&, FVector&);
};
