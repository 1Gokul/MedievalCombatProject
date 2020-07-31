// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()


protected:

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

public:

	void GameModeOnly();
	void GameAndUIMode();

	bool bUIWidgetCurrentlyActive();

	/** For Player HUD	*/

	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	bool bHUDVisible;

	/** For Enemy health bar */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	bool bEnemyHealthBarVisible;

	FVector EnemyLocation;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	/** Pause Menu*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	/** Item Pickup prompt */
	bool bItemInteractPromptVisible;

	// Display the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayItemInteractPrompt(const FString& ItemName, float Weight, int32 Value, FName InteractText);

	// Remove the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveItemInteractPrompt();

	/** Search prompt */
	bool bSearchPromptVisible;

	// Display the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplaySearchPrompt(const FString& ItemName, FName SearchText);

	// Remove the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveSearchPrompt();

	/** Inventory Menu*/

	bool bInventoryMenuVisible;

	void ToggleInventoryMenu(class UInventoryComponent* InventoryComponent);

	//Parameter needed to set Inventory in BP
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayInventoryMenu(class UInventoryComponent* InventoryComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveInventoryMenu();

};
