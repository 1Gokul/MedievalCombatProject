// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Main.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void GameModeOnly();
	void GameAndUIMode();

	bool bUIWidgetCurrentlyActive();

	// Display the HUD when the Character is active
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayHUD();

	// Remove the HUD when the Character is idle for more than IdleTimerLimit seconds.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void HideHUD();

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	/** Item Pickup prompt */


	// Display the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayItemInteractPrompt(const FString& ItemName, float Weight, int32 Value, FName InteractText);

	// Remove the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveItemInteractPrompt();

	/** Search prompt */


	// Display the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplaySearchPrompt(const FString& ItemName, FName SearchText);

	// Remove the pickup prompt widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveSearchPrompt();

	/** Inventory Menu*/


	void ToggleInventoryMenu(class UInventoryComponent* InventoryComponent);

	//Parameter needed to set Inventory in BP
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayInventoryMenu(class UInventoryComponent* InventoryComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveInventoryMenu();

	void SetEnemyLocation(const FVector& InLocation) { EnemyLocation = InLocation; }

	bool IsAnyMenuVisible();

	FORCEINLINE bool IsHUDVisible() const { return bHUDVisible; }

	FORCEINLINE bool IsInventoryMenuVisible() const { return bInventoryMenuVisible; }

	FORCEINLINE bool IsPauseMenuVisible() const { return bPauseMenuVisible; }

	FORCEINLINE bool IsSearchPromptVisible() const { return bSearchPromptVisible; }

	FORCEINLINE bool IsItemInteractPromptVisible() const { return bItemInteractPromptVisible; }


protected:

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	/** For Player HUD	*/

	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;


	/** For Enemy health bar */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;


	/** Pause Menu*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;


private:
	bool bHUDVisible;

	bool bEnemyHealthBarVisible;

	bool bPauseMenuVisible;

	bool bItemInteractPromptVisible;

	bool bSearchPromptVisible;

	bool bInventoryMenuVisible;

	// Enemy health bar will be displayed at this location.
	FVector EnemyLocation;
};
