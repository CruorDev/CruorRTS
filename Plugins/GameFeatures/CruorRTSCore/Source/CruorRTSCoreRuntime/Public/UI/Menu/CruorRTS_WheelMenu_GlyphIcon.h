//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_WheelMenu_GlyphIcon.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCruorRTS_WheelMenu_GlyphIcon
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EboronradeUnitLeader|IconData")
	FString Glyph;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EboronradeUnitLeader|IconData")
	FText Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EboronradeUnitLeader|IconData")
	FText Description;
	
};
