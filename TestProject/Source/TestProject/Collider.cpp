// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ColliderMovementComponent.h"

// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));

	//Attach SphereComponent to RootComponent
	SphereComponent->SetupAttachment(GetRootComponent());
	SphereComponent->InitSphereRadius(40.0f);
	SphereComponent->SetCollisionProfileName("Pawn");

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());

	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->RelativeRotation = FRotator(-45.0f, 0.0f, 0.0f);
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 3.0f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

	OurMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("MovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);

}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
	return OurMovementComponent;
}

void ACollider::MoveForward(float Value)
{
	FVector Forward = GetActorForwardVector();

	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(Forward * Value, false);
	}

}

void ACollider::MoveRight(float Value)
{
	FVector Right = GetActorRightVector();

	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(Right * Value, false);
	}
}

