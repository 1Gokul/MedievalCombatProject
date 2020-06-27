// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item.h"
#include "InventoryComponent.generated.h"

// Make Blueprints bind this to update the UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);


USTRUCT(BlueprintType)
struct FSlotStructure
{
	GENERATED_BODY()

	//Constructor
	FSlotStructure();

	/**
 *	Used for the Inventory.Find() function.
 *	Overrides the default '==' operator.
 *	Therefore, if the SlotStructure Object passed as a parameter has Name == "None" it will be treated as an Empty Item.
 */
	bool operator==(const FSlotStructure& arg) const
	{
		return (arg.ItemStructure.ItemDisplayName == FName("None"));
	}

	/** Reference to the FItemStructure variable of the Item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
	struct FItemStructure ItemStructure;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
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

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	TArray<AItem*> Items;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	
	/** Resize the Inventory to NumberOfSlots. */
	void PrepareInventory();

	/** Creates a stack for a particular Item.
	 * 	@param ContentToAdd - The Item SlotStructure to add.
	 */
	void CreateStack(FSlotStructure ContentToAdd);

	/**
	 * Adds the SlotStructure of the Item into the Inventory.
	 * @param ContentToAdd - The Item SlotStructure to add.
	 *
	 * returns true if ContentToAdd was successfully added to the Inventory.
	 */
	 bool AddToInventory(FSlotStructure ContentToAdd);
	
};
