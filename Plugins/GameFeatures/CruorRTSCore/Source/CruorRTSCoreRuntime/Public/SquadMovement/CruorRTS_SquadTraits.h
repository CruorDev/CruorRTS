//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "CruorRTS_SquadTraits.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "CruorRTSSquad"))
class CRUORRTSCORERUNTIME_API UCruorRTS_SquadTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
		
	UPROPERTY(EditAnywhere)
	FCruorRTS_SquadMemberMovementSettingsFragment MemberMovementSettingsFragment;
};
