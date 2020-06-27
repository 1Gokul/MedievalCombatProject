// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Item.h"

//Sets default values for a FSlotStructure object
FSlotStructure::FSlotStructure()
{
	 Quantity = 0;
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	InventoryName = FText::FromString("Backpack");

	// Inventory Menu layout - 3 rows with 7 columns each.
	NumberOfSlots = 21;
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

void UInventoryComponent::CreateStack(FSlotStructure ContentToAdd)
{
	int32 Index;

	// Empty Slot Object
	FSlotStructure Slot;

	// Find the position of the first empty element of the Inventory Array.
	Inventory.Find(Slot, Index);


	// Insert the ContentToAdd at that index.
	Inventory.Insert(ContentToAdd, Index);
}

bool UInventoryComponent::AddToInventory(FSlotStructure ContentToAdd)
{
	// If the Item is Stackable
	if(ContentToAdd.ItemStructure.bIsStackable)
	{
		return true;
	}
	else
	{
		CreateStack(ContentToAdd);
		return true;
	}

	return false;
}
