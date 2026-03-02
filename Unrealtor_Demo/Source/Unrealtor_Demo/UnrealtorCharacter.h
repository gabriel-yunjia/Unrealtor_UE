#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnrealtorCharacter.generated.h"

class UCameraComponent;

UCLASS()
class AUnrealtorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUnrealtorCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float LookSensitivity = 1.f;

	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;
};
