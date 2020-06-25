// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Item.h"

//Sets default values for a FSlotStructure objecct
FSlotStructure::FSlotStructure()
{
	
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	InventoryName = FText::FromString("Backpack");
	
	NumberOfSlots = 20;
}

bool UInventoryComponent::AddItem(AItem* Item)
{
	if(Items.Num() >= NumberOfSlots || !Item)
	{
		return false;
	}

	
	Item->OwningInventory = this;
	Item->World = GetWorld();
	Items.Add(Item);

	//Update UI
	OnInventoryUpdated.Broadcast();

	return true;
}

bool UInventoryComponent::RemoveItem(AItem* Item)
{

	if(Item)
	{
		Item->OwningInventory = nullptr;
		Item->World = nullptr;
		Items.RemoveSingle(Item);

		//Update UI
		OnInventoryUpdated.Broadcast();

		return true;
	}

	return false;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}
