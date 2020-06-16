// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "Main.h"

void AFoodItem::Use(AMain* Main)
{
	if(Main)
	{
		Main->Health += HealthToHeal;
	}
}
