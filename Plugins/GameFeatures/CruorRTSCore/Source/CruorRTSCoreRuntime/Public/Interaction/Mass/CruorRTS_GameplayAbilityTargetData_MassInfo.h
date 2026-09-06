// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "CruorRTS_GameplayAbilityTargetData_MassInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCruorRTS_GameplayAbilityTargetData_MassInfo : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = MassTargetData)
	FMassEntityHandle SourceEntityHandle;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = MassTargetData)
	FMassEntityHandle SquadAnchorEntity;
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FCruorRTS_GameplayAbilityTargetData_MassInfo::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return TEXT("FCruorRTS_GameplayAbilityTargetData_MassInfo");
	}
	
	CRUORRTSCORERUNTIME_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FCruorRTS_GameplayAbilityTargetData_MassInfo> : public TStructOpsTypeTraitsBase2<FCruorRTS_GameplayAbilityTargetData_MassInfo>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
