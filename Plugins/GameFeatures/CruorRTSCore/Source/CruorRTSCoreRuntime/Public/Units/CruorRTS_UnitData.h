// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CruorRTS_UnitData.generated.h"


#define UE_API CRUORRTSCORERUNTIME_API

class UCruorRTS_HeroUnitPawnData;
class UCruorRTS_MassUnitData;
class UCruorRTS_UnitLeaderPawnData;
/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Cruor Unit Data", ShortTooltip = "Data asset used to define units to spawn."))
class UCruorRTS_UnitData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_UnitData(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cruor|UnitData")
	TArray<TSoftObjectPtr<UCruorRTS_HeroUnitPawnData>> HeroUnits;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cruor|UnitData")
	TArray<TSoftObjectPtr<UCruorRTS_MassUnitData>> MassUnits;	
};
#undef UE_API
