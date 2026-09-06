// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MassSpawnerTypes.h"
#include "CruorRTS_MassUnitData.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "CruorRTS Mass Unit Data", ShortTooltip = "Data asset used to define a MassEntity based Unit."))
class UCruorRTS_MassUnitData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_MassUnitData(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS|MassUnit", meta = (Tooltip = "Treat Proportion as the Total Number of Units to spawn."))
	TArray<FMassSpawnedEntityType> MassUnits;	
	
};

#undef UE_API
