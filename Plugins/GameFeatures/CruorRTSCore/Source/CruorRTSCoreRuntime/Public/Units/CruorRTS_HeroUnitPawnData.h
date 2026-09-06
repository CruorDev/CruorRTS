// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CruorRTS_HeroUnitPawnData.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class AAIController;
class ULyraPawnData;
/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "CruorRTS_ Hero Unit Pawn Data", ShortTooltip = "Data asset used to define a Hero Unit Pawn and their Controller"))
class UCruorRTS_HeroUnitPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_HeroUnitPawnData(const FObjectInitializer& ObjectInitializer);
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|Pawn")
	FName HeroUnitName;
	
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|PawnData")
	TObjectPtr<ULyraPawnData> PawnData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|PawnController")
	TSubclassOf<AAIController> PawnController;
};

#undef UE_API
