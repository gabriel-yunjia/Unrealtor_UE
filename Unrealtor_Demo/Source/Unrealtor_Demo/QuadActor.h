#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealtorTypes.h"
#include "QuadActor.generated.h"

class AMatchActor;
class USphereComponent;
class UAlignmentPointComponent;

UCLASS()
class AQuadActor : public AActor
{
	GENERATED_BODY()

public:
	AQuadActor();

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	EQuadSide QuadSide = EQuadSide::Left;

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	TObjectPtr<AMatchActor> OwnerMatch;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	float TriggerRadius = 500.f;

	// World-space mesh vertices, cached on BeginPlay.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	TArray<FVector> CachedVertices;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> QuadMesh;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAlignmentPointComponent> DefaultAlignPoint0;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAlignmentPointComponent> DefaultAlignPoint1;

	void CacheWorldVertices();
};
