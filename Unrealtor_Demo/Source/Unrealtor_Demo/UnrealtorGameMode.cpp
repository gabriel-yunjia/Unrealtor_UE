#include "UnrealtorGameMode.h"
#include "UnrealtorPlayerController.h"
#include "UnrealtorCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AUnrealtorGameMode::AUnrealtorGameMode()
{
	DefaultPawnClass = AUnrealtorCharacter::StaticClass();
	PlayerControllerClass = AUnrealtorPlayerController::StaticClass();
}

void AUnrealtorGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* NewPC = UGameplayStatics::CreatePlayer(this, -1, true);
	if (!NewPC)
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealtorGameMode: Failed to create Player 2"));
	}

	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		CachePlayerReferences();

		// IMC stays on Player 1 permanently. Input handlers route to the active character.
		if (Player1Controller)
		{
			AssignIMCToPlayer(Player1Controller, true);
		}
	});
}

void AUnrealtorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AUnrealtorPlayerController* PC = Cast<AUnrealtorPlayerController>(NewPlayer);
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	int32 Index = LP ? LP->GetLocalPlayerIndex() : 0;
	PC->PlayerIndex = Index;

	if (Index == 0)
		Player1Controller = PC;
	else
		Player2Controller = PC;
}

void AUnrealtorGameMode::CachePlayerReferences()
{
	if (Player1Controller)
		Player1Character = Cast<AUnrealtorCharacter>(Player1Controller->GetPawn());
	if (Player2Controller)
		Player2Character = Cast<AUnrealtorCharacter>(Player2Controller->GetPawn());

	UE_LOG(LogTemp, Log, TEXT("Unrealtor: P1=%s P2=%s"),
		Player1Character ? *Player1Character->GetName() : TEXT("null"),
		Player2Character ? *Player2Character->GetName() : TEXT("null"));
}

void AUnrealtorGameMode::SwitchActivePlayer()
{
	int32 NewIndex = (ActivePlayerIndex == 0) ? 1 : 0;
	AUnrealtorPlayerController* NewPC = GetPlayerController(NewIndex);

	if (!NewPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchActivePlayer: Player %d controller is null, aborting."), NewIndex);
		return;
	}

	ActivePlayerIndex = NewIndex;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Active Player: %d"), ActivePlayerIndex));
	}
}

void AUnrealtorGameMode::AssignIMCToPlayer(AUnrealtorPlayerController* PC, bool bAdd)
{
	if (!PC || !IMC_Player) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	if (bAdd)
		Subsystem->AddMappingContext(IMC_Player, 0);
	else
		Subsystem->RemoveMappingContext(IMC_Player);
}

AUnrealtorPlayerController* AUnrealtorGameMode::GetPlayerController(int32 Index) const
{
	return (Index == 0) ? Player1Controller : Player2Controller;
}

AUnrealtorCharacter* AUnrealtorGameMode::GetPlayerCharacter(int32 Index) const
{
	return (Index == 0) ? Player1Character : Player2Character;
}
