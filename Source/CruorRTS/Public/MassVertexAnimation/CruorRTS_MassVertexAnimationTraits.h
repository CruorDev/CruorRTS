//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_MassVertexAnimationFragments.h"
#include "MassEntityTraitBase.h"
#include "CruorRTS_MassVertexAnimationTraits.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTS_API UCruorRTS_MassVertexAnimationTraits : public UMassEntityTraitBase
{
	GENERATED_BODY()	
		
public:	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Animation")
	FCruorRTS_MassVertexAnimationParametersFragment MassVertexAnimationParameters;
};
