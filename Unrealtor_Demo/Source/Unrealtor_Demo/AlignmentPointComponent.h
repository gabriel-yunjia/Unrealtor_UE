#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AlignmentPointComponent.generated.h"

UCLASS(ClassGroup = "Puzzle", meta = (BlueprintSpawnableComponent))
class UAlignmentPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAlignmentPointComponent();

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	int32 PointIndex = 0;
};
