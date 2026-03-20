//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_WheelMenu_GlyphIcon.h"
#include "CruorRTS_WheelMenu_GlyphIconData.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

/**
 * Container Asset for Wheel Menu Icons and their associated Names/Descriptions
 */
UCLASS(BlueprintType, Const)
class UCruorRTS_WheelMenu_GlyphIconData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_WheelMenu_GlyphIconData(const FObjectInitializer& ObjectInitializer);
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FCruorRTS_WheelMenu_GlyphIcon> GlyphIcons;
};

#undef UE_API
