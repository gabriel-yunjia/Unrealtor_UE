#include "AlignmentPointComponent.h"

UAlignmentPointComponent::UAlignmentPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsOnUpdateTransform = false;

#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}
