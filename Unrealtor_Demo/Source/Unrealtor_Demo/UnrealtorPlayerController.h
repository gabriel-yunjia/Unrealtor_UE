// Per-player controller. Binds Enhanced Input actions and forwards to the possessed character.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UnrealtorPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class AUnrealtorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 PlayerIndex = -1;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_SwitchPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> AlignmentHUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> AlignmentHUDWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bShowAlignmentFrame = false;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float AlignmentAutoSubmitNormalized = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float AlignmentCloseness = 0.f;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetAlignmentHUDState(bool bInShowAlignmentFrame, float InAutoSubmitNormalized, float InCloseness);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResetAlignmentHUDState();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnAlignmentHUDStateUpdated(bool bInShowAlignmentFrame, float InAutoSubmitNormalized, float InCloseness);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSwitchPlayer(const FInputActionValue& Value);
	void TryCreateAlignmentHUDWidget();

	int32 AlignmentHUDRetryCount = 0;
};
