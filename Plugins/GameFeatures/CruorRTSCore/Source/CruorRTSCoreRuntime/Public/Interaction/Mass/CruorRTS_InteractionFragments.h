// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_GameplayAbilityTargetData_MassInfo.h"
#include "Avoidance/MassAvoidanceFragments.h"
#include "Interaction/InteractionOption.h"
#include "UI/Menu/CruorRTS_WheelMenu_GlyphIconData.h"
#include "CruorRTS_InteractionFragments.generated.h"

/**
 * This is a stub.
 */

USTRUCT()
struct FCruorRTS_InteractionFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FInteractionOption InteractionOption;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> InnerWheelMenuGlyphIconData;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> OuterWheelMenuGlyphIconData;
};

template <>
struct TMassFragmentTraits<FCruorRTS_InteractionFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
};

