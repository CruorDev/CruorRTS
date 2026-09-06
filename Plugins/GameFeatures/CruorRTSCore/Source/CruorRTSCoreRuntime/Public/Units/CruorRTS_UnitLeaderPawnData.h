// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CruorRTS_UnitLeaderPawnData.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class AAIController;
class ULyraPawnData;
/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "CruorRTS Unit Leader Pawn Data", ShortTooltip = "Data asset used to define a Unit Leader Pawn and their Controller"))
class UCruorRTS_UnitLeaderPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UE_API explicit UCruorRTS_UnitLeaderPawnData(const FObjectInitializer& ObjectInitializer);
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|Pawn")
	FName UnitLeaderName;
	
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|PawnData")
	TObjectPtr<ULyraPawnData> PawnData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS_UnitLeader|PawnController")
	TSubclassOf<AAIController> PawnController;
};

#undef UE_API
