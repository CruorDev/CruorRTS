// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "MassEntityTemplate.h"
#include "MassSpawnerTypes.h"
#include "Components/ActorComponent.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "CruorRTS_UnitSpawnerComponent.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class UCruorRTS_HeroUnitPawnData;
class UCruorRTS_UnitData;
class UCruorRTS_MassUnitData;
class UCruorRTS_SquadData;
struct FStreamableHandle;
class ULyraExperienceDefinition;
class AAIController;
class UCruorRTS_UnitLeaderPawnData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCruorRTS_UnitSpawnerOnSpawningFinishedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCruorRTS_UnitSpawnerOnDespawningFinishedEvent);

UCLASS(MinimalAPI, Blueprintable, meta=(BlueprintSpawnableComponent), hidecategories = (Object, Actor, Input, Rendering, LOD, Cooking, Collision, HLOD, Partition))
class UCruorRTS_UnitSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API explicit UCruorRTS_UnitSpawnerComponent(const FObjectInitializer& ObjectInitializer);
	
	// UActorComponent overrides
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	// ~UActorComponent overrides
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CruorRTS|CommanderUnits")
	TSoftObjectPtr<UCruorRTS_UnitData> Units;
	
	
protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category=Gameplay)
	void ServerSpawnUnits();

	/** Sequential loading process */
	void StartAsyncLoading();
	void OnUnitDataLoaded();
	void OnUnitsLoaded();
	
	void SpawnAllUnits();
	
	virtual void SpawnOneHeroUnit(const UCruorRTS_HeroUnitPawnData* NewHeroUnitData);
	
	UFUNCTION()
	void OnSpawnedPawnDestroyed(AActor* DestroyedPawn);
	
private:
	void UpdateUnitsTeam();
	
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
	UFUNCTION()
	void OnTeamAssigned(UObject* TeamAgent, const int32 OldTeamID, const int32 NewTeamID);
	
	static APawn* SpawnHeroUnitFromClass(
		const UObject* WorldContextObject,
		const UCruorRTS_HeroUnitPawnData* LoadedPawnData,
		const FVector& Location,
		const FRotator& Rotation,
		bool bNoCollisionFail,
		AActor* PawnOwner);
	
protected:
	
	UPROPERTY()
	TArray<TObjectPtr<AAIController>> UnitLeaderControllers;	
	
	UPROPERTY()
	TArray<TObjectPtr<AAIController>> HeroUnitControllers;	
	
private:
	
	FGenericTeamId TeamID;
	
	//////////////////////////////////////
	/// * Mass Spawner Implementation  ///
	/// * Inspired by AMassSpawner     ///
	//////////////////////////////////////
	
public:
	
	/**
	 * Starts the spawning of all the agent types of this spawner
	 */
	
	UE_API void SpawnMassUnits(const TSoftObjectPtr<UCruorRTS_MassUnitData> MassUnitData);

	/**
	 * Despawn all mass agents that was spawned by this spawner
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	UE_API void DoDespawning();

	/**
	 * Despawn all mass agent that was spawned by this spawner, except EntitiesToIgnore.
	 *
	 * Any EntitiesToIgnore previously spawned by this spawner will remain spawned and tracked by this spawner.   
	 */
	UE_API void DoDespawning(TConstArrayView<FMassEntityHandle> EntitiesToIgnore);

	/**
	 * If given entity has been spawned by this MassSpawner instance then it will get destroyed and all the bookkeeping 
	 * updated. Otherwise, the call has no effect.
	 * @return true if the entity got removed. False otherwise.
	 */
	UE_API bool DespawnEntity(const FMassEntityHandle Entity);	

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	UE_API int32 GetCount() const;

	/** Called once SpawnUnitMembers completes and all entities have been spawned. */
	UPROPERTY(BlueprintAssignable)
	FCruorRTS_UnitSpawnerOnSpawningFinishedEvent CruorRTS_UnitSpawnerOnSpawningFinishedEvent;

	/** Called once DoDespawning completes and all mass agents spawned by this spawner have been despawned. */
	UPROPERTY(BlueprintAssignable)
	FCruorRTS_UnitSpawnerOnDespawningFinishedEvent OnDespawningFinishedEvent; 
	
protected:
	UE_API void OnMassUnitsSpawnDataGenerationFinished(TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results, TArray<FMassSpawnedEntityType> EntityTypes, AActor* Owner);
	UE_API void SpawnGeneratedMassUnits(TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results, TArray<FMassSpawnedEntityType> EntityTypes);

	UE_API UMassProcessor* GetPostSpawnProcessor(TSubclassOf<UMassProcessor> ProcessorClass);
	
protected:

	struct FSpawnedEntities
	{
		FMassEntityTemplateID TemplateID;
		TArray<FMassEntityHandle> Entities;
	};

	int32 Count;
	int32 HeroUnitCount;
	int32 MassUnitCount;

	/** Array of entity spawn generators. These define where to spawn entities. */
	UPROPERTY(EditAnywhere, Category = "Mass|Spawn")
	FMassSpawnDataGenerator SpawnDataGenerator;

	UPROPERTY(Category = "Mass|Spawn", EditAnywhere)
	uint32 bAutoSpawnOnBeginPlay : 1;

	/** By default TickSchematics will be appended to the simulation's schematics. If this property is set to true the
	 *  TickSchematics will override the original simulation schematics */
	UPROPERTY(Category = "Mass|Simulation", EditAnywhere)
	uint32 bOverrideSchematics : 1;

	UPROPERTY()
	TArray<TObjectPtr<UMassProcessor>> PostSpawnProcessors;

	FDelegateHandle SimulationStartedHandle;

	TArray<FSpawnedEntities> AllSpawnedEntities;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;

	int32 CurrentLeaderIndex;
	int32 CurrentHeroUnitIndex;
	int32 CurrentMassUnitIndex;
	
	//////////////////////////////////////
	/// ~ Mass Spawner Implementation  ///
	//////////////////////////////////////
};

#undef UE_API
