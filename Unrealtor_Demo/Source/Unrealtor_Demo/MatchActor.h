// Core puzzle actor. Owns a Left and Right quad, detects screen-space alignment,
// and solves the match when both sides hold alignment for AutoSubmitDuration.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealtorTypes.h"
#include "MatchActor.generated.h"

class AQuadActor;
class AUnrealtorGameMode;

UCLASS()
class AMatchActor : public AActor
{
	GENERATED_BODY()

public:
	AMatchActor();

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	TObjectPtr<AQuadActor> LeftQuad;

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	TObjectPtr<AQuadActor> RightQuad;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Thresholds")
	float XThresholdPercent = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Thresholds")
	float YThresholdPercent = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Thresholds")
	float FacingAngleThreshold = 20.f;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Timer")
	float AutoSubmitDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Timer")
	float AutoSubmitDecayRate = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle|State")
	bool bIsSolved = false;

	void OnPlayerEnteredSide(EQuadSide Side);
	void OnPlayerExitedSide(EQuadSide Side);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	bool bLeftPlayerNearby = false;
	bool bRightPlayerNearby = false;
	bool bLeftAligned = false;
	bool bRightAligned = false;
	float Closeness = 0.f;
	float AutoSubmitTimer = 0.f;

	TObjectPtr<AUnrealtorGameMode> CachedGM;

	void EvaluateAlignment(float DeltaTime);
};
