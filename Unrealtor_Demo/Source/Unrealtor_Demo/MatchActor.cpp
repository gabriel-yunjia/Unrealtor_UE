#include "MatchActor.h"
#include "QuadActor.h"
#include "UnrealtorGameMode.h"
#include "UnrealtorPlayerController.h"
#include "UnrealtorCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AMatchActor::AMatchActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AMatchActor::BeginPlay()
{
	Super::BeginPlay();
	CachedGM = GetWorld()->GetAuthGameMode<AUnrealtorGameMode>();

	UE_LOG(LogTemp, Warning, TEXT("MatchActor %s BeginPlay: GM=%s LeftQuad=%s RightQuad=%s"),
		*GetName(),
		CachedGM ? TEXT("OK") : TEXT("NULL"),
		LeftQuad ? *LeftQuad->GetName() : TEXT("NULL"),
		RightQuad ? *RightQuad->GetName() : TEXT("NULL"));

	if (LeftQuad)
	{
		UE_LOG(LogTemp, Warning, TEXT("  LeftQuad verts=%d side=%d ownerMatch=%s"),
			LeftQuad->CachedVertices.Num(),
			(int32)LeftQuad->QuadSide,
			LeftQuad->OwnerMatch ? *LeftQuad->OwnerMatch->GetName() : TEXT("NULL"));
	}
	if (RightQuad)
	{
		UE_LOG(LogTemp, Warning, TEXT("  RightQuad verts=%d side=%d ownerMatch=%s"),
			RightQuad->CachedVertices.Num(),
			(int32)RightQuad->QuadSide,
			RightQuad->OwnerMatch ? *RightQuad->OwnerMatch->GetName() : TEXT("NULL"));
	}
}

void AMatchActor::OnPlayerEnteredSide(EQuadSide Side)
{
	UE_LOG(LogTemp, Warning, TEXT("MatchActor %s: OnPlayerEnteredSide %s"),
		*GetName(), (Side == EQuadSide::Left) ? TEXT("LEFT") : TEXT("RIGHT"));

	if (Side == EQuadSide::Left)
		bLeftPlayerNearby = true;
	else
		bRightPlayerNearby = true;
}

void AMatchActor::OnPlayerExitedSide(EQuadSide Side)
{
	UE_LOG(LogTemp, Warning, TEXT("MatchActor %s: OnPlayerExitedSide %s"),
		*GetName(), (Side == EQuadSide::Left) ? TEXT("LEFT") : TEXT("RIGHT"));

	if (Side == EQuadSide::Left)
		bLeftPlayerNearby = false;
	else
		bRightPlayerNearby = false;
}

void AMatchActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSolved) return;

	if (!CachedGM || !LeftQuad || !RightQuad)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				static_cast<int32>(GetUniqueID()) + 1, 0.f, FColor::Red,
				FString::Printf(TEXT("Match BROKEN: GM=%s LQ=%s RQ=%s"),
					CachedGM ? TEXT("ok") : TEXT("NULL"),
					LeftQuad ? TEXT("ok") : TEXT("NULL"),
					RightQuad ? TEXT("ok") : TEXT("NULL")));
		}
		return;
	}

	// Show player distances to quads every frame.
	AUnrealtorCharacter* P1 = CachedGM->Player1Character;
	AUnrealtorCharacter* P2 = CachedGM->Player2Character;

	float DistL = P1 ? FVector::Dist(P1->GetActorLocation(), LeftQuad->GetActorLocation()) : -1.f;
	float DistR = P2 ? FVector::Dist(P2->GetActorLocation(), RightQuad->GetActorLocation()) : -1.f;

	// Drive proximity directly from distance — overlap events are unreliable.
	bLeftPlayerNearby = P1 && (DistL <= LeftQuad->TriggerRadius);
	bRightPlayerNearby = P2 && (DistR <= RightQuad->TriggerRadius);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()) + 2, 0.f, FColor::Cyan,
			FString::Printf(TEXT("P1 dist to LQ=%.0f (r=%.0f)  P2 dist to RQ=%.0f (r=%.0f)"),
				DistL, LeftQuad->TriggerRadius,
				DistR, RightQuad->TriggerRadius));

		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()) + 1, 0.f,
			(bLeftPlayerNearby && bRightPlayerNearby) ? FColor::Green : FColor::White,
			FString::Printf(TEXT("Match: LeftNearby=%s RightNearby=%s  LQ verts=%d RQ verts=%d"),
				bLeftPlayerNearby ? TEXT("YES") : TEXT("no"),
				bRightPlayerNearby ? TEXT("YES") : TEXT("no"),
				LeftQuad->CachedVertices.Num(),
				RightQuad->CachedVertices.Num()));
	}

	// Vertex color interpolates red → green based on last frame's Closeness (one-frame lag, imperceptible).
	FColor VertColor(
		static_cast<uint8>(FMath::RoundToInt(255.f * (1.f - Closeness))),
		static_cast<uint8>(FMath::RoundToInt(255.f * Closeness)),
		0
	);

	for (int32 i = 0; i < LeftQuad->CachedVertices.Num(); ++i)
	{
		DrawDebugPoint(GetWorld(), LeftQuad->CachedVertices[i], 8.f, VertColor, false, 0.f);
	}
	for (int32 i = 0; i < RightQuad->CachedVertices.Num(); ++i)
	{
		DrawDebugPoint(GetWorld(), RightQuad->CachedVertices[i], 8.f, VertColor, false, 0.f);
	}

	if (!bLeftPlayerNearby || !bRightPlayerNearby)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);
		return;
	}
	if (!P1 || !P2)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);
		return;
	}

	EvaluateAlignment(DeltaTime);
}

void AMatchActor::EvaluateAlignment(float DeltaTime)
{
	AUnrealtorPlayerController* PC1 = CachedGM->Player1Controller;
	AUnrealtorPlayerController* PC2 = CachedGM->Player2Controller;
	const bool bLeftAlignedPrevFrame = bLeftAligned;
	const bool bRightAlignedPrevFrame = bRightAligned;
	if (!PC1 || !PC2)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("EvalAlign: PC1 or PC2 null"));
		return;
	}

	AUnrealtorCharacter* P1 = CachedGM->Player1Character;
	AUnrealtorCharacter* P2 = CachedGM->Player2Character;

	int32 ViewW1, ViewH1, ViewW2, ViewH2;
	PC1->GetViewportSize(ViewW1, ViewH1);
	PC2->GetViewportSize(ViewW2, ViewH2);

	if (ViewW1 <= 0 || ViewH1 <= 0 || ViewW2 <= 0 || ViewH2 <= 0)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
				FString::Printf(TEXT("EvalAlign: Bad viewport: V1=%dx%d V2=%dx%d"), ViewW1, ViewH1, ViewW2, ViewH2));
		return;
	}

	// --- Step 1: Project all vertices to screen space ---
	TArray<FVector2D> LeftScreenVerts;
	for (const FVector& V : LeftQuad->CachedVertices)
	{
		FVector2D ScreenPos;
		if (PC1->ProjectWorldLocationToScreen(V, ScreenPos, true))
		{
			LeftScreenVerts.Add(ScreenPos);
		}
	}

	TArray<FVector2D> RightScreenVerts;
	for (const FVector& V : RightQuad->CachedVertices)
	{
		FVector2D ScreenPos;
		if (PC2->ProjectWorldLocationToScreen(V, ScreenPos, true))
		{
			RightScreenVerts.Add(ScreenPos);
		}
	}

	if (LeftScreenVerts.Num() < 2 || RightScreenVerts.Num() < 2)
	{
		PushAlignmentUI(false, false, 0.f, 0.f);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
				FString::Printf(TEXT("EvalAlign: Not enough screen verts L=%d R=%d"), LeftScreenVerts.Num(), RightScreenVerts.Num()));
		return;
	}

	// --- Step 2: Find inner edge vertices ---
	LeftScreenVerts.Sort([](const FVector2D& A, const FVector2D& B) { return A.X > B.X; });
	FVector2D TopLeft, BottomLeft;
	if (LeftScreenVerts[0].Y > LeftScreenVerts[1].Y)
	{
		TopLeft = LeftScreenVerts[0];
		BottomLeft = LeftScreenVerts[1];
	}
	else
	{
		TopLeft = LeftScreenVerts[1];
		BottomLeft = LeftScreenVerts[0];
	}

	RightScreenVerts.Sort([](const FVector2D& A, const FVector2D& B) { return A.X < B.X; });
	FVector2D TopRight, BottomRight;
	if (RightScreenVerts[0].Y > RightScreenVerts[1].Y)
	{
		TopRight = RightScreenVerts[0];
		BottomRight = RightScreenVerts[1];
	}
	else
	{
		TopRight = RightScreenVerts[1];
		BottomRight = RightScreenVerts[0];
	}

	// --- Step 3: Alignment check ---
	// UE5 ProjectWorldLocationToScreen returns viewport-local coords.
	// Left viewport seam = right edge (X=ViewW). Right viewport seam = left edge (X=0).
	float SeamX_L = ViewW1 / 2.f;
	float SeamX_R = 0.f;

	float TopLeftDiff = SeamX_L - TopLeft.X;
	float BottomLeftDiff = SeamX_L - BottomLeft.X;
	float TopRightDiff = SeamX_R - TopRight.X;
	float BottomRightDiff = SeamX_R - BottomRight.X;

	float TopYDiff = FMath::Abs(TopLeft.Y - TopRight.Y);
	float BottomYDiff = FMath::Abs(BottomLeft.Y - BottomRight.Y);

	float XThreshold = ViewW1 * XThresholdPercent / 100.f;
	float YThreshold = ViewH1 * YThresholdPercent / 100.f;

	// --- Step 4: Facing angle check ---
	FVector DirToP1 = (P1->GetActorLocation() - LeftQuad->GetActorLocation()).GetSafeNormal();
	float DotLeft = FVector::DotProduct(DirToP1, LeftQuad->GetActorForwardVector());
	float AngleLeft = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotLeft, -1.f, 1.f)));

	FVector DirToP2 = (P2->GetActorLocation() - RightQuad->GetActorLocation()).GetSafeNormal();
	float DotRight = FVector::DotProduct(DirToP2, RightQuad->GetActorForwardVector());
	float AngleRight = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotRight, -1.f, 1.f)));

	// --- Step 5: Per-side alignment ---
	bLeftAligned =
		FMath::Abs(TopLeftDiff) <= XThreshold &&
		FMath::Abs(BottomLeftDiff) <= XThreshold &&
		AngleLeft <= FacingAngleThreshold;

	bRightAligned =
		FMath::Abs(TopRightDiff) <= XThreshold &&
		FMath::Abs(BottomRightDiff) <= XThreshold &&
		AngleRight <= FacingAngleThreshold;

	bool bFullMatch =
		bLeftAligned && bRightAligned &&
		TopYDiff <= YThreshold &&
		BottomYDiff <= YThreshold;

	// --- Step 6: Closeness ---
	float XError = (FMath::Abs(TopLeftDiff) + FMath::Abs(BottomLeftDiff) +
		FMath::Abs(TopRightDiff) + FMath::Abs(BottomRightDiff)) / 4.f;
	float YError = (TopYDiff + BottomYDiff) / 2.f;
	float AngleError = (AngleLeft + AngleRight) / 2.f;

	float MaxXError = XThreshold * 3.f;
	float MaxYError = YThreshold * 3.f;
	float MaxAngleError = FacingAngleThreshold * 3.f;

	float XFactor = 1.f - FMath::Clamp(XError / MaxXError, 0.f, 1.f);
	float YFactor = 1.f - FMath::Clamp(YError / MaxYError, 0.f, 1.f);
	float AngleFactor = 1.f - FMath::Clamp(AngleError / MaxAngleError, 0.f, 1.f);
	Closeness = (XFactor + YFactor + AngleFactor) / 3.f;

	// --- Step 7: Auto-submit ---
	if (bFullMatch)
	{
		AutoSubmitTimer += DeltaTime;
		if (AutoSubmitTimer >= AutoSubmitDuration)
		{
			bIsSolved = true;
			PushAlignmentUI(false, false, 1.f, 1.f);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
					FString::Printf(TEXT("MATCH SOLVED: %s"), *GetName()));
			}
			return;
		}
	}
	else
	{
		AutoSubmitTimer -= DeltaTime * AutoSubmitDecayRate;
		AutoSubmitTimer = FMath::Max(AutoSubmitTimer, 0.f);
	}

	const float AutoSubmitNormalized = (AutoSubmitDuration > KINDA_SMALL_NUMBER)
		? FMath::Clamp(AutoSubmitTimer / AutoSubmitDuration, 0.f, 1.f)
		: 0.f;
	
	bool bShowLeftUI = false;
	bool bShowRightUI = false;

	if (bLeftAligned && bRightAligned)
	{
		if (bFullMatch)
		{
			bShowLeftUI = true;
			bShowRightUI = true;
		}
		else
		{
			// Partial state: show one side only. Prefer the side that reached alignment first;
			// if both were reached together, prefer the side with lower seam error this frame.
			if (bLeftAlignedPrevFrame && !bRightAlignedPrevFrame)
			{
				bShowLeftUI = true;
			}
			else if (!bLeftAlignedPrevFrame && bRightAlignedPrevFrame)
			{
				bShowRightUI = true;
			}
			else
			{
				const float LeftSeamError = FMath::Abs(TopLeftDiff) + FMath::Abs(BottomLeftDiff);
				const float RightSeamError = FMath::Abs(TopRightDiff) + FMath::Abs(BottomRightDiff);
				bShowLeftUI = LeftSeamError <= RightSeamError;
				bShowRightUI = !bShowLeftUI;
			}
		}
	}
	else
	{
		bShowLeftUI = bLeftAligned;
		bShowRightUI = bRightAligned;
	}

	PushAlignmentUI(bShowLeftUI, bShowRightUI, AutoSubmitNormalized, Closeness);

	// Full alignment debug.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()), 0.f, bFullMatch ? FColor::Green : FColor::Yellow,
			FString::Printf(TEXT("Align: L=%s R=%s Close=%.2f Timer=%.1f/%.1f"),
				bLeftAligned ? TEXT("Y") : TEXT("N"),
				bRightAligned ? TEXT("Y") : TEXT("N"),
				Closeness,
				AutoSubmitTimer, AutoSubmitDuration));

		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()) + 3, 0.f, FColor::Yellow,
			FString::Printf(TEXT("Screen: TL=(%.0f,%.0f) BL=(%.0f,%.0f) TR=(%.0f,%.0f) BR=(%.0f,%.0f)  Mid=%0.f/%0.f"),
				TopLeft.X, TopLeft.Y, BottomLeft.X, BottomLeft.Y,
				TopRight.X, TopRight.Y, BottomRight.X, BottomRight.Y,
				SeamX_L, SeamX_R));

		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()) + 4, 0.f, FColor::Yellow,
			FString::Printf(TEXT("XDiff: TL=%.0f BL=%.0f TR=%.0f BR=%.0f  thresh=%.0f  |  YDiff: T=%.0f B=%.0f thresh=%.0f  |  Angle: L=%.0f R=%.0f thresh=%.0f"),
				TopLeftDiff, BottomLeftDiff, TopRightDiff, BottomRightDiff, XThreshold,
				TopYDiff, BottomYDiff, YThreshold,
				AngleLeft, AngleRight, FacingAngleThreshold));

		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()) + 5, 0.f, FColor::Yellow,
			FString::Printf(TEXT("Viewport: V1=%dx%d V2=%dx%d  |  Projected verts: L=%d R=%d"),
				ViewW1, ViewH1, ViewW2, ViewH2,
				LeftScreenVerts.Num(), RightScreenVerts.Num()));
	}
}

void AMatchActor::PushAlignmentUI(bool bShowLeftFrame, bool bShowRightFrame, float AutoSubmitNormalized, float InCloseness)
{
	if (!CachedGM)
	{
		return;
	}

	if (AUnrealtorPlayerController* PC1 = CachedGM->Player1Controller)
	{
		PC1->SetAlignmentHUDState(bShowLeftFrame, AutoSubmitNormalized, InCloseness);
	}

	if (AUnrealtorPlayerController* PC2 = CachedGM->Player2Controller)
	{
		PC2->SetAlignmentHUDState(bShowRightFrame, AutoSubmitNormalized, InCloseness);
	}
}
