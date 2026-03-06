//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Components/ActorComponent.h"
#include "CruorRTS_UnitLeaderSpawnerComponent.generated.h"

#define  UE_API CRUORRTSCORERUNTIME_API

class AAIController;
class UCruorRTS_UnitLeaderSpawnerPawnData;
class ULyraExperienceDefinition;

/* @CruorRTS_CodeAttribution This class is taken in whole or part from NanceDevDiaries see: https://www.github.com/NanceDevDiaries/ */

UCLASS(MinimalAPI, Blueprintable, ClassGroup=(CruorRTS_PlayerPawn), meta=(BlueprintSpawnableComponent))
class UCruorRTS_UnitLeaderSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API explicit UCruorRTS_UnitLeaderSpawnerComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS|Pawn")
	TArray<TSoftObjectPtr<UCruorRTS_UnitLeaderSpawnerPawnData>> UnitLeaders; 

protected:
	UE_API virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category=Gameplay)
	void ServerSpawnUnitLeaders();
	
	virtual void SpawnOneUnitLeader(TSoftObjectPtr<UCruorRTS_UnitLeaderSpawnerPawnData> SpawningUnitLeaderData);
	
	UFUNCTION()
	void OnSpawnedPawnDestroyed(AActor* DestroyedPawn);
	
private:
	
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
	UFUNCTION()
	void OnTeamAssigned(UObject* TeamAgent, int32 OldTeamID, int32 NewTeamID);

	static APawn* SpawnUnitLeaderFromClass(
		const UObject* WorldContextObject, 
		const UCruorRTS_UnitLeaderSpawnerPawnData* LoadedPawnData, 
		const FVector& Location,
		const FRotator& Rotation, 
		bool bNoCollisionFail, 
		AActor* PawnOwner);
	
protected:
	
	TArray<TObjectPtr<AAIController>> UnitLeaderControllers;	
	
private:
	
	FGenericTeamId TeamID;
};

#undef UE_API
