// GameMode for the Unrealtor demo. Spawns two local players, manages split-screen,
// and handles Tab-switching of the active input target.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UnrealtorGameMode.generated.h"

class AUnrealtorPlayerController;
class AUnrealtorCharacter;
class UInputMappingContext;

UCLASS()
class AUnrealtorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUnrealtorGameMode();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Player;

	UPROPERTY(BlueprintReadOnly, Category = "Players")
	TObjectPtr<AUnrealtorPlayerController> Player1Controller;

	UPROPERTY(BlueprintReadOnly, Category = "Players")
	TObjectPtr<AUnrealtorPlayerController> Player2Controller;

	UPROPERTY(BlueprintReadOnly, Category = "Players")
	TObjectPtr<AUnrealtorCharacter> Player1Character;

	UPROPERTY(BlueprintReadOnly, Category = "Players")
	TObjectPtr<AUnrealtorCharacter> Player2Character;

	UPROPERTY(BlueprintReadOnly, Category = "Players")
	int32 ActivePlayerIndex = 0;

	void SwitchActivePlayer();

	AUnrealtorPlayerController* GetPlayerController(int32 Index) const;
	AUnrealtorCharacter* GetPlayerCharacter(int32 Index) const;

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void AssignIMCToPlayer(AUnrealtorPlayerController* PC, bool bAdd);
	void CachePlayerReferences();
};
