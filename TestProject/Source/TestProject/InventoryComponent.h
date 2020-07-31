// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Item.h"
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
 
	bool operator==(const FSlotStructure& arg) const
	{
		return (arg.ItemStructure.ItemDisplayName == FName("None"));
	}	*/

	/** Reference to the FItemStructure variable of the Item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
	struct FItemStructure ItemStructure;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
	int32 Quantity;
};

static bool operator==(const FSlotStructure& arg1, const FSlotStructure& arg2)
{
	return (arg1.ItemStructure.ItemDisplayName == arg2.ItemStructure.ItemDisplayName);
}

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	FString InventoryName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	int32 NumberOfSlots = 21;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlotStructure")
	TArray<FSlotStructure> Inventory;

protected:
	// Called when the game starts
	void BeginPlay() override;

public:


	/** Resize the Inventory to NumberOfSlots. */
	void PrepareInventory();

	/** Creates a stack for a particular Item.
	 * 	@param ContentToAdd The Item SlotStructure to add.
	 *
	 * 	@returns True if a Stack for the Item could be created successfully
	 * 				and there was an available slot in the Inventory to do so.
	 */
	bool CreateStack(FSlotStructure ContentToAdd);

	/**
	 * Adds the SlotStructure of the Item into the Inventory.
	 * @param ContentToAdd The Item SlotStructure to add.
	 *
	 * @returns True if ContentToAdd was successfully added to the Inventory.
	 */
	UFUNCTION(BlueprintCallable)
	bool AddToInventory(FSlotStructure ContentToAdd);

	/**
	 * Adds a Stackable Item into its respective ItemStack in the Inventory.
	 *
	 * @param SlotStructure The Item to add
	 * @param SlotIndex The position of the ItemStack in the Inventory.
	 *
	 * @returns True if the Item was successfully added to the Stack.
	 */

	bool AddToStack(FSlotStructure SlotStructure, int32 SlotIndex);

	/** 
	 * 	Receives a Query for an Item and checks if the Item is present
	 * 	in the inventory in the required amount.
	 *
	 * 	@param QueryItem The Item to be searched for
	 * 	@param QueryAmount The amount of QueryItems required
	 *
	 * 	@returns True if the QueryItem is available in the required QueryAmount.
	 */
	UFUNCTION(BlueprintCallable)
	bool InventoryQuery(TSubclassOf<AItem> QueryItem, int32 QueryAmount);

	/**
	 * This function removes the equipped weapon or shield from the Player if
	 * the Weapon/Shield to be added is the same as the Current Weapon/Shield
	 * that the Player has equipped.
	 */

	UFUNCTION(BlueprintCallable)
	bool ShouldUnequipWeaponOrShield(TSubclassOf<AItem> ItemToCheck, AMain* Main);

};
