// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bPauseMenuVisible = false;
	bInventoryMenuVisible = false;

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}

	// TODO: Change to be visible only when the level starts because it will have to be hidden when a Main Menu is added.
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);
	bHUDVisible = true;

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);

		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}

		FVector2D Alignment(0.0f, 0.0f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);

		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Update position of Enemy health bar depending on the Enemy's current position */
	if (EnemyHealthBar)
	{
		FVector2D PositionInViewport;

		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);

		PositionInViewport.Y -= 90.0f;


		FVector2D SizeInViewport(300.0f, 25.0f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly;

	SetInputMode(InputModeGameOnly);
}

void AMainPlayerController::GameAndUIMode()
{
	FInputModeGameAndUI InputModeGameAndUI;

	SetInputMode(InputModeGameAndUI);
}

bool AMainPlayerController::bUIWidgetCurrentlyActive()
{
	return(bInventoryMenuVisible || bPauseMenuVisible);
}


void AMainPlayerController::DisplayEnemyHealthBar()
{
	
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
	
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	// Hide the HUD
	if(bHUDVisible){
		HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
		bHUDVisible = false;
	}
	
	if (PauseMenu)
	{
		bPauseMenuVisible = true;

		UGameplayStatics::SetGamePaused(GetWorld(), true);

		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		GameAndUIMode();
		
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	// Show the HUD
	if(!bHUDVisible){
		HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		bHUDVisible = true;
	}
	
	if (PauseMenu)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		//Visibility is set in blueprint

		GameModeOnly();

		bShowMouseCursor = false;

		bPauseMenuVisible = false;
	}
	
}

void AMainPlayerController::DisplayInventoryMenu_Implementation(UInventoryComponent* InventoryComponent)
{
	// Hide the HUD
	if(bHUDVisible){
		HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
		bHUDVisible = false;
	}
	
	if (InventoryMenu)
	{
		bInventoryMenuVisible = true;

		UGameplayStatics::SetGamePaused(GetWorld(), true);

		//InventoryMenu->SetVisibility(ESlateVisibility::Visible);

		GameAndUIMode();
		
		bShowMouseCursor = true;
	}
}


void AMainPlayerController::RemoveInventoryMenu_Implementation()
{
	// Show the HUD
	if(!bHUDVisible){
		HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		bHUDVisible = true;
	}
	
	if (InventoryMenu)
	{
		bInventoryMenuVisible = false;
		
		UGameplayStatics::SetGamePaused(GetWorld(), false);			

		GameModeOnly();

		bShowMouseCursor = false;

		
	}
}


