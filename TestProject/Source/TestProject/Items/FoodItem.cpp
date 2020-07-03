// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "InventoryComponent.h"
#include "Main.h"

bool AFoodItem::UseItem(AMain* Main)
{	
	Main->IncrementHealth(HealthToHeal);

	return true;
}
