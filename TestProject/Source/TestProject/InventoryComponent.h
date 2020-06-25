// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

// Make Blueprints bind this to update the UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

USTRUCT(BlueprintType)
struct FSlotStructure
{
	GENERATED_BODY()
	
	//Constructor
	FSlotStructure();
	
	struct FItemStructure* ItemStructure;
	int32 Quantity;	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	FText InventoryName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	int32 NumberOfSlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
	TArray<FSlotStructure> Inventory;

	bool AddItem(class AItem* Item);
	bool RemoveItem(AItem* Item);  
	

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	TArray<AItem*> Items;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
		
};
