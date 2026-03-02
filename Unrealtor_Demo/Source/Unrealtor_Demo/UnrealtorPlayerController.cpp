#include "UnrealtorPlayerController.h"
#include "UnrealtorCharacter.h"
#include "UnrealtorGameMode.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

void AUnrealtorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC) return;

	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AUnrealtorPlayerController::HandleMove);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AUnrealtorPlayerController::HandleMove);
	}
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AUnrealtorPlayerController::HandleLook);
	}
	if (IA_SwitchPlayer)
	{
		EIC->BindAction(IA_SwitchPlayer, ETriggerEvent::Started, this, &AUnrealtorPlayerController::HandleSwitchPlayer);
	}
}

void AUnrealtorPlayerController::HandleMove(const FInputActionValue& Value)
{
	AUnrealtorGameMode* GM = GetWorld()->GetAuthGameMode<AUnrealtorGameMode>();
	if (!GM) return;

	AUnrealtorCharacter* TargetChar = GM->GetPlayerCharacter(GM->ActivePlayerIndex);
	if (!TargetChar) return;

	AUnrealtorPlayerController* TargetPC = GM->GetPlayerController(GM->ActivePlayerIndex);
	if (!TargetPC) return;

	FVector2D Input = Value.Get<FVector2D>();

	FRotator YawRotation(0.f, TargetPC->GetControlRotation().Yaw, 0.f);
	FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	TargetChar->AddMovementInput(Forward, Input.Y);
	TargetChar->AddMovementInput(Right, Input.X);
}

void AUnrealtorPlayerController::HandleLook(const FInputActionValue& Value)
{
	AUnrealtorGameMode* GM = GetWorld()->GetAuthGameMode<AUnrealtorGameMode>();
	if (!GM) return;

	AUnrealtorPlayerController* TargetPC = GM->GetPlayerController(GM->ActivePlayerIndex);
	AUnrealtorCharacter* TargetChar = GM->GetPlayerCharacter(GM->ActivePlayerIndex);
	if (!TargetPC || !TargetChar) return;

	FVector2D Input = Value.Get<FVector2D>();
	float Sens = TargetChar->LookSensitivity;

	TargetPC->AddYawInput(Input.X * Sens);
	TargetPC->AddPitchInput(-Input.Y * Sens);
}

void AUnrealtorPlayerController::HandleSwitchPlayer(const FInputActionValue& Value)
{
	AUnrealtorGameMode* GM = GetWorld()->GetAuthGameMode<AUnrealtorGameMode>();
	if (GM)
	{
		GM->SwitchActivePlayer();
	}
}
