#include "QuadActor.h"
#include "AlignmentPointComponent.h"
#include "MatchActor.h"
#include "UnrealtorCharacter.h"
#include "UnrealtorPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AQuadActor::AQuadActor()
{
	PrimaryActorTick.bCanEverTick = false;

	QuadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuadMesh"));
	RootComponent = QuadMesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetSphereRadius(TriggerRadius);
	TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerSphere->SetGenerateOverlapEvents(true);

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuadActor::OnTriggerBeginOverlap);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AQuadActor::OnTriggerEndOverlap);

	DefaultAlignPoint0 = CreateDefaultSubobject<UAlignmentPointComponent>(TEXT("AlignPoint_0"));
	DefaultAlignPoint0->SetupAttachment(RootComponent);
	DefaultAlignPoint0->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	DefaultAlignPoint0->PointIndex = 0;

	DefaultAlignPoint1 = CreateDefaultSubobject<UAlignmentPointComponent>(TEXT("AlignPoint_1"));
	DefaultAlignPoint1->SetupAttachment(RootComponent);
	DefaultAlignPoint1->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	DefaultAlignPoint1->PointIndex = 1;
}

void AQuadActor::BeginPlay()
{
	Super::BeginPlay();
	TriggerSphere->SetSphereRadius(TriggerRadius);
	CacheWorldVertices();
}

void AQuadActor::CacheWorldVertices()
{
	CachedVertices.Empty();

	TArray<UAlignmentPointComponent*> Points;
	GetComponents<UAlignmentPointComponent>(Points);

	if (Points.Num() > 0)
	{
		Points.Sort([](const UAlignmentPointComponent& A, const UAlignmentPointComponent& B)
		{
			return A.PointIndex < B.PointIndex;
		});

		for (const UAlignmentPointComponent* Pt : Points)
		{
			CachedVertices.Add(Pt->GetComponentLocation());
		}

		UE_LOG(LogTemp, Log, TEXT("QuadActor %s [%s]: Cached %d vertices from AlignmentPointComponents."),
			*GetName(), (QuadSide == EQuadSide::Left) ? TEXT("Left") : TEXT("Right"),
			CachedVertices.Num());
		return;
	}

	// Fallback: build 8 corners from the mesh AABB.
	FBoxSphereBounds Bounds = QuadMesh->CalcBounds(QuadMesh->GetComponentTransform());
	FVector Min = Bounds.Origin - Bounds.BoxExtent;
	FVector Max = Bounds.Origin + Bounds.BoxExtent;

	CachedVertices.Add(FVector(Min.X, Min.Y, Min.Z));
	CachedVertices.Add(FVector(Min.X, Min.Y, Max.Z));
	CachedVertices.Add(FVector(Min.X, Max.Y, Min.Z));
	CachedVertices.Add(FVector(Min.X, Max.Y, Max.Z));
	CachedVertices.Add(FVector(Max.X, Min.Y, Min.Z));
	CachedVertices.Add(FVector(Max.X, Min.Y, Max.Z));
	CachedVertices.Add(FVector(Max.X, Max.Y, Min.Z));
	CachedVertices.Add(FVector(Max.X, Max.Y, Max.Z));

	UE_LOG(LogTemp, Log, TEXT("QuadActor %s [%s]: Cached %d vertices from AABB fallback. Min=(%.0f,%.0f,%.0f) Max=(%.0f,%.0f,%.0f)"),
		*GetName(), (QuadSide == EQuadSide::Left) ? TEXT("Left") : TEXT("Right"),
		CachedVertices.Num(),
		Min.X, Min.Y, Min.Z, Max.X, Max.Y, Max.Z);
}

void AQuadActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUnrealtorCharacter* Char = Cast<AUnrealtorCharacter>(OtherActor);
	if (!Char || !OwnerMatch) return;

	AUnrealtorPlayerController* PC = Cast<AUnrealtorPlayerController>(Char->GetController());
	if (!PC) return;

	bool bCorrectPlayer =
		(QuadSide == EQuadSide::Left && PC->PlayerIndex == 0) ||
		(QuadSide == EQuadSide::Right && PC->PlayerIndex == 1);

	UE_LOG(LogTemp, Warning, TEXT("QuadActor %s [%s]: Overlap BEGIN — %s (PlayerIndex=%d) Correct=%s"),
		*GetName(), (QuadSide == EQuadSide::Left) ? TEXT("Left") : TEXT("Right"),
		*Char->GetName(), PC->PlayerIndex, bCorrectPlayer ? TEXT("YES") : TEXT("NO"));

	if (bCorrectPlayer)
	{
		OwnerMatch->OnPlayerEnteredSide(QuadSide);
	}
}

void AQuadActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUnrealtorCharacter* Char = Cast<AUnrealtorCharacter>(OtherActor);
	if (!Char || !OwnerMatch) return;

	AUnrealtorPlayerController* PC = Cast<AUnrealtorPlayerController>(Char->GetController());
	if (!PC) return;

	bool bCorrectPlayer =
		(QuadSide == EQuadSide::Left && PC->PlayerIndex == 0) ||
		(QuadSide == EQuadSide::Right && PC->PlayerIndex == 1);

	if (bCorrectPlayer)
	{
		OwnerMatch->OnPlayerExitedSide(QuadSide);
	}
}
