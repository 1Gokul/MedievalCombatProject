// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "Main.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Items/Shield.h"
#include "Items/Weapon.h"

//Sets default values for a FSlotStructure object
FSlotStructure::FSlotStructure()
{
	Quantity = 0;
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	InventoryName = FText::FromString("Inventory");
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	PrepareInventory();
}


void UInventoryComponent::PrepareInventory()
{
	Inventory.SetNum(NumberOfSlots);
}

bool UInventoryComponent::CreateStack(FSlotStructure ContentToAdd)
{
	// First check if there is space to create a stack i.e. check if an empty slot is available

	bool bHasSpace = false;

	for (FSlotStructure Element : Inventory)
	{
		if (Element.Quantity <= 0)
		{
			bHasSpace = true;
			break;
		}
	}

	if (bHasSpace)
	{
		int32 Index;

		// Empty Slot Object
		FSlotStructure Slot;

		// Find the position of the first empty element of the Inventory Array.
		Inventory.Find(Slot, Index);

		if (Inventory.IsValidIndex(Index))
		{
			Inventory[Index] = ContentToAdd;
			UE_LOG(LogTemp, Warning, TEXT("Index = %i"), Index);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory Index is not Valid! = %i"), Index);
		}
	}

	return bHasSpace;
	// Insert the ContentToAdd at that index.
	//Inventory.Insert(ContentToAdd, Index);
}

bool UInventoryComponent::AddToInventory(FSlotStructure ContentToAdd)
{
	// If the Item is Stackable
	if (ContentToAdd.ItemStructure.bIsStackable)
	{
		// If an Item of this type already exists in the Inventory, add it to the stack.

		int32 Index = -1;
		bool bCanAddToStack = false;

		int i = 0; //Gets incremented in the loop below.
		for (FSlotStructure& Element : Inventory)
		{
			if (
				(Element.ItemStructure.ItemClass == ContentToAdd.ItemStructure.ItemClass)	// If the Item class matches
				&& (Element.Quantity < ContentToAdd.ItemStructure.MaxStackSize
				)		// If the stack is not completely filled
			)
			{
				// Find the location of the Element
				Index = i; /*Inventory.Find(Element); */
				bCanAddToStack = true;
			}
			++i;
		}

		if (bCanAddToStack)
		{
			AddToStack(ContentToAdd, Index);
			UE_LOG(LogTemp, Warning, TEXT("Added to stack! Index = %i"), Index);

			return true;
		}
		else
		{
			bool Success = CreateStack(ContentToAdd);

			if (Success)
			{
				UE_LOG(LogTemp, Warning, TEXT("Created stack!"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No space to create stack!"));
			}

			return Success;
		}
	}
	bool Success = CreateStack(ContentToAdd);

	if (Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Created stack!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No space to create stack!"));
	}

	return Success;
}

bool UInventoryComponent::AddToStack(FSlotStructure SlotStructure, int32 SlotIndex)
{
	// MaxStackSize of SlotStructure's ItemStructure
	const int32 MaxStackSize = Inventory[SlotIndex].ItemStructure.MaxStackSize;

	// If adding the new Items would cause the stack to overflow
	if (Inventory[SlotIndex].Quantity + SlotStructure.Quantity > MaxStackSize)
	{
		// Current Quantity of the Stack before adding the Items
		const int32 CurrentQuantity = Inventory[SlotIndex].Quantity;

		/**
		 *	1. Fill up the current Stack to its MaxStackSize.
		 *	2. Get the amount of Items left in the SlotStructure
		 *		( SlotStructure.Quantity - (MaxStackSize - CurrentQuantity).
		 *	3. Add the remaining Items to the Inventory. The Inventory will create a new Stack for the remaining Items.
		 *
		 *	This process will loop again if the remaining number of Items is again greater than the MaxStackSize.
		 */

		// 1.
		Inventory[SlotIndex].Quantity = MaxStackSize;

		// 2.
		SlotStructure.Quantity -= MaxStackSize - CurrentQuantity;

		// 3.
		AddToInventory(SlotStructure);
	}
		// If it doesn't
	else
	{
		// Increase the ItemQuantity by the amount of new Items i.e. Add the new Items into the Stack.
		Inventory[SlotIndex].Quantity += SlotStructure.Quantity;
	}

	return true;
}

bool UInventoryComponent::InventoryQuery(TSubclassOf<AItem> QueryItem, int32 QueryAmount)
{
	int32 RunningTotal = 0;

	for (FSlotStructure Element : Inventory)
	{
		if (Element.ItemStructure.ItemClass == QueryItem)
		{
			RunningTotal += Element.Quantity;
		}
	}

	return (RunningTotal >= QueryAmount);
}

bool UInventoryComponent::ShouldUnequipWeaponOrShield(TSubclassOf<AItem> ItemToCheck, AMain* Main)
{
	if (Main->CurrentWeapon)
	{
		if (ItemToCheck == Main->CurrentWeapon->GetClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("Same as weapon!!"));

			AWeapon* Temp = Main->CurrentWeapon;
			Main->SetCurrentWeapon(nullptr);

			Temp->Destroy();

			Main->bIsWeaponDrawn = false;

			return true;
		}
	}
	else if (Main->EquippedShield/* && Shield*/)
	{
		if (ItemToCheck == Main->EquippedShield->GetClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("Same as shield!!"));

			AShield* Temp = Main->EquippedShield;
			Main->SetEquippedShield(nullptr);

			Temp->Destroy();

			//Main->CheckPlayerStatus();

			return true;
		}
	}

		return false;
	
}
