// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSave.h"

UGameSave::UGameSave()
{
	PlayerName = TEXT("Default");

	UserIndex = 0;

	CharacterStats.EquippedWeaponName = TEXT("");
	CharacterStats.LevelName = TEXT("");
}
