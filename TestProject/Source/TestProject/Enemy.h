// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8 {
	EMS_Idle			UMETA(DisplayName = "Idle"),
	EMS_MoveToTarget	UMETA(DisplayName = "MoveToTarget"),
	EMS_Attacking		UMETA(DisplayName = "Attacking"),
	EMS_Dead			UMETA(DisplayName = "Dead"),

	EMS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class TESTPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		EEnemyMovementStatus EnemyMovementStatus;

	/** Enemy will become aggressive and chase the player 
	*	if the player overlaps with this sphere	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere;

	/** Enemy will become attack the player
	*	if the player overlaps with this sphere	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		 USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		class AAIController* AIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		class AMain* CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DamageIfBlocked;

	/** Particles emitted when the enemy gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	/** Sound that gets emitted when the enemy gets hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI ")
	class USoundCue* HitSound;

	/** Sound that gets emitted when the enemy swings at the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound;

	/** Collision Box attached to the left hand's weapon to generate overlap events 
	*	if the weapon comes in contact with the Player during an attack. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* LeftCombatCollision;

	/** Collision Box attached to the right hand's weapon to generate overlap events
	*	if the weapon comes in contact with the Player during an attack. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* RightCombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
		class UAnimMontage* CombatMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	bool bHasValidTarget;

	FTimerHandle AttackTimer;


	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DeathDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
		float MinAttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
		float MaxAttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
		TSubclassOf<UDamageType> DamageTypeClass;

	/** Socket names based on Attack Names*/

	FName CurrentAttackTipSocket;
	FName Attack1_TipSocket;
	FName Attack2_TipSocket;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }
	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }

	/**Called if an AgroSphere Overlap Event starts. */
	UFUNCTION()
		virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**Called if an CombatSphere Overlap Event starts. */
	UFUNCTION()
		virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the AgroSphere Overlap Event ends */
	UFUNCTION()
		virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Called when the CombatSphere Overlap Event ends */
	UFUNCTION()
		virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Main);
	
	void InflictDamageOnMain(AMain* Char, bool bHitFromBehind);

	/**Called if a Combat Overlap Event starts. */
	UFUNCTION()
		void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the Combat Overlap Event ends. */
	UFUNCTION()
		void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void ActivateCollisionLeft();

	UFUNCTION(BlueprintCallable)
		void ActivateCollisionRight();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollisionLeft();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollisionRight();

	void Attack();

	UFUNCTION(BlueprintCallable)
		void AttackEnd();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void DeathEnd();

	bool Alive(); 

	void Disappear();
};
	
