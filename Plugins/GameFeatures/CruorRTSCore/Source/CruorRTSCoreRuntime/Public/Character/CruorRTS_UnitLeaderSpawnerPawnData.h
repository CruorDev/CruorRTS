//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CruorRTS_UnitLeaderSpawnerPawnData.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class AAIController;
class ULyraPawnData;
/**
 * Data asset used to define a Unit Leader Pawn and their Controller
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "CruorRTS Unit Leader Pawn Data", ShortTooltip = "Data asset used to define a Unit Leader Pawn and their Controller"))
class UCruorRTS_UnitLeaderSpawnerPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_UnitLeaderSpawnerPawnData(const FObjectInitializer& ObjectInitializer);
	
public:
	
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTSUnitLeader|PawnData")
	TObjectPtr<ULyraPawnData> PawnData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTSUnitLeader|PawnController")
	TSubclassOf<AAIController> PawnController;
};
