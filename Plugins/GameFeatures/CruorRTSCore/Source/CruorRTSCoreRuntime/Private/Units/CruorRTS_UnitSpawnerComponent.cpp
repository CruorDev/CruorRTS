// Copyright Cruor enf., All Rights Reserved.


#include "Units/CruorRTS_UnitSpawnerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Async/ParallelFor.h"
#include "MassEntityConfigAsset.h"
#include "MassExecutor.h"
#include "MassObserverNotificationTypes.h"
#include "MassProcessingContext.h"
#include "MassSimulationSubsystem.h"
#include "MassSpawner.h"
#include "MassSpawnerSubsystem.h"
#include "MassStateTreeFragments.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/CruorRTS_CharacterWithAbilitiesBase.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Units/CruorRTS_UnitLeaderPawnData.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "CruorRTS/CruorRTSLogChannels.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Teams/LyraTeamSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Units/CruorRTS_MassUnitData.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "Steering/MassSteeringFragments.h"
#include "Units/CruorRTS_UnitData.h"
#include "Units/CruorRTS_HeroUnitPawnData.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "SquadMovement/CruorRTS_SquadMovementSubsystem.h"
#include "UObject/FastReferenceCollector.h"
#include "VisualLogger/VisualLogger.h"

#define COLLISION_LANDSCAPE ECC_GameTraceChannel6

namespace CruorRTS_UnitSpawner
{
	static float GroundTraceDistance = 10000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("CruorRTS_UnitSpawner.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace up when updating the Spawned Pawn's z-height."), ECVF_Cheat);
}

UCruorRTS_UnitSpawnerComponent::UCruorRTS_UnitSpawnerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoSpawnOnBeginPlay = true;
	bOverrideSchematics = false;
}

void UCruorRTS_UnitSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();	

	// Listen for the experience load to complete
	const AGameStateBase* GameState = GetWorld()->GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));

	if (AActor* Owner = GetOwner())
	{
		if (ILyraTeamAgentInterface* OwnerInterface = Cast<ILyraTeamAgentInterface>(Owner))
		{
			OwnerInterface->GetOnTeamIndexChangedDelegate()->AddDynamic(this, &ThisClass::OnTeamAssigned);
		}
		else
		{
			UE_LOG(LogCruorRTS, Warning, TEXT("CruorRTSUnitSpawner: OwningActor %s does not implement ILyraTeamAgentInterface"), *Owner->GetName());
		}
	}
}

void UCruorRTS_UnitSpawnerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SimulationStartedHandle.IsValid())
	{
		UMassSimulationSubsystem::GetOnSimulationStarted().Remove(SimulationStartedHandle);
		SimulationStartedHandle.Reset();
	}
	
	if (StreamingHandle.IsValid() && StreamingHandle->IsActive())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}

	DoDespawning();
	Super::EndPlay(EndPlayReason);
}

void UCruorRTS_UnitSpawnerComponent::PostLoad()
{
	Super::PostLoad();
	
	if (SpawnDataGenerator.GeneratorClass)
	{
		SpawnDataGenerator.GeneratorInstance = NewObject<UMassEntitySpawnDataGeneratorBase>(this, SpawnDataGenerator.GeneratorClass);
		SpawnDataGenerator.GeneratorClass = nullptr;
		(void)MarkPackageDirty();
	}
}

void UCruorRTS_UnitSpawnerComponent::BeginDestroy()
{
	DoDespawning();
	
	if (StreamingHandle.IsValid() && StreamingHandle->IsActive())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	
	Super::BeginDestroy();
}

#if WITH_SERVER_CODE
void UCruorRTS_UnitSpawnerComponent::ServerSpawnUnits_Implementation()
{
	// if (!bAutoSpawnOnBeginPlay || Squads.IsNull())
	// {
	// 	return;
	// }
	
	if (!bAutoSpawnOnBeginPlay || Units.IsNull()) return;

	if (const UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(GetWorld()); 
		MassSimulationSubsystem && MassSimulationSubsystem->IsSimulationStarted())
	{
		StartAsyncLoading();
	}
	else if (!SimulationStartedHandle.IsValid())
	{
		TWeakObjectPtr<UCruorRTS_UnitSpawnerComponent> WeakThis(this);
		SimulationStartedHandle = UMassSimulationSubsystem::GetOnSimulationStarted().AddLambda([WeakThis](const UWorld* InWorld)
		{
			if (WeakThis.IsValid() && WeakThis->GetWorld() == InWorld)
			{
				WeakThis->StartAsyncLoading();
			}
		});
	}
}

void UCruorRTS_UnitSpawnerComponent::StartAsyncLoading()
{
	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	
	if (Units.IsValid())
	{
		OnUnitDataLoaded();
	}
	else
	{
		StreamingHandle = StreamableManager.RequestAsyncLoad(Units.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ThisClass::OnUnitDataLoaded));
	}
}

void UCruorRTS_UnitSpawnerComponent::OnUnitDataLoaded()
{
	UCruorRTS_UnitData* LoadedUnits = Units.Get();
	if (!LoadedUnits)
	{
		UE_LOG(LogCruorRTS, Error, TEXT("Failed to load Squads asset."));
		return;
	}

	TArray<FSoftObjectPath> AssetsToLoad;
	for (const auto& HeroUnit : LoadedUnits->HeroUnits)
	{
		if (!HeroUnit.IsNull())
		{
			AssetsToLoad.Add(HeroUnit.ToSoftObjectPath());
		}			
	}
	
	for (const auto& MassUnit : LoadedUnits->MassUnits)
	{
		if (!MassUnit.IsNull())
		{
			AssetsToLoad.Add(MassUnit.ToSoftObjectPath());
		}			
	}

	if (AssetsToLoad.Num() > 0)
	{
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		StreamingHandle = StreamableManager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &ThisClass::OnUnitsLoaded));
	}
	else
	{
		OnUnitsLoaded();
	}
}

void UCruorRTS_UnitSpawnerComponent::OnUnitsLoaded()
{
	SpawnAllUnits();
}

void UCruorRTS_UnitSpawnerComponent::SpawnAllUnits()
{
	UCruorRTS_UnitData* LoadedUnits = Units.Get();
	if (!LoadedUnits) return;
	
	HeroUnitCount = LoadedUnits->HeroUnits.Num();
		
	for (const auto& MassUnit : LoadedUnits->MassUnits)
	{
		const UCruorRTS_MassUnitData* Data = MassUnit.Get();		
		MassUnitCount += Data->MassUnits.Num();
	}
	
	for (const auto& HeroUnit : LoadedUnits->HeroUnits)
	{
		if (const UCruorRTS_HeroUnitPawnData* HeroData = HeroUnit.Get())
		{
			SpawnOneHeroUnit(HeroData);
		}
	}	
	
	for (const auto& MassUnit : LoadedUnits->MassUnits)
	{
		SpawnMassUnits(MassUnit);
	}
}

void UCruorRTS_UnitSpawnerComponent::SpawnOneHeroUnit(const UCruorRTS_HeroUnitPawnData* NewHeroUnitData)
{
	if (!NewHeroUnitData)
	{
		return;
	}

	FVector SpawnLocation = GetOwner()->GetActorLocation();
	if (const UWorld* World = GetWorld())
	{
		const FRotator CurrentRotation = FRotator(0.f, 360.f / HeroUnitCount * CurrentHeroUnitIndex, 0.f);
		const FVector StartLocation = CurrentRotation.Vector() * 150.f + GetOwner()->GetActorLocation();

		const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UnitLeaderTrace), false, GetOwner());
		
		if (FHitResult HitResultAboveRoot; World->LineTraceSingleByChannel(HitResultAboveRoot, StartLocation + GetOwner()->GetActorUpVector() * CruorRTS_UnitSpawner::GroundTraceDistance, StartLocation - GetOwner()->GetActorUpVector() * CruorRTS_UnitSpawner::GroundTraceDistance, COLLISION_LANDSCAPE, QueryParams))
		{
			SpawnLocation = HitResultAboveRoot.Location;
		}
	}
	
	APawn* SpawnedHeroUnit = SpawnHeroUnitFromClass(GetWorld(), NewHeroUnitData, SpawnLocation, GetOwner()->GetActorRotation(), true, GetOwner());
	if (!SpawnedHeroUnit) return;

	bool bWantsPlayerState = true;
	if (AAIController* AIController = Cast<AAIController>(SpawnedHeroUnit->Controller))
	{
		bWantsPlayerState = AIController->bWantsPlayerState;
		HeroUnitControllers.Add(AIController);
	}
	
	if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedHeroUnit))
	{
		AActor* AbilityOwner = bWantsPlayerState ? SpawnedHeroUnit->GetPlayerState() : Cast<AActor>(SpawnedHeroUnit);
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner))
		{
			PawnExtComp->InitializeAbilitySystem(Cast<ULyraAbilitySystemComponent>(AbilitySystemComponent), AbilityOwner);
		}
	}

	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{
		if (const ACruorRTS_PlayerPawn* PlayerPawn = GetOwner<ACruorRTS_PlayerPawn>())
		{
			TeamID = PlayerPawn->GetGenericTeamId();
			TeamSubsystem->ChangeTeamForActor(SpawnedHeroUnit->Controller, TeamID);
		}
	}
	
	SpawnedHeroUnit->OnDestroyed.AddDynamic(this, &ThisClass::OnSpawnedPawnDestroyed);
	CurrentHeroUnitIndex++;
}

void UCruorRTS_UnitSpawnerComponent::OnSpawnedPawnDestroyed(AActor* DestroyedPawn)
{
	if (GetOwner()->HasAuthority() && DestroyedPawn)
	{
		if (const APawn* Pawn = Cast<APawn>(DestroyedPawn))
		{
			UnitLeaderControllers.Remove(Cast<AAIController>(Pawn->GetController()));
		}
	}
}

void UCruorRTS_UnitSpawnerComponent::UpdateUnitsTeam()
{
	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{		
		for (const TObjectPtr<AAIController>& UnitLeader : UnitLeaderControllers)
		{		
			TeamSubsystem->ChangeTeamForActor(UnitLeader, TeamID);
		}		
	}
}

#endif // WITH_SERVER_CODE

void UCruorRTS_UnitSpawnerComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (GetOwner()->HasAuthority())
	{
		ServerSpawnUnits();		
	}
#endif // WITH_SERVER_CODE	
}

void UCruorRTS_UnitSpawnerComponent::OnTeamAssigned(UObject* TeamAgent, const int32 OldTeamID, const int32 NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		TeamID = NewTeamID;	
		UpdateUnitsTeam();
	}
}

APawn* UCruorRTS_UnitSpawnerComponent::SpawnHeroUnitFromClass(
	const UObject* WorldContextObject,
	const UCruorRTS_HeroUnitPawnData* LoadedPawnData, 
	const FVector& Location, const FRotator& Rotation,
	bool bNoCollisionFail,
	AActor* PawnOwner)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !LoadedPawnData || !LoadedPawnData->PawnData)
	{
		return nullptr;
	}

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Owner = PawnOwner;
	ActorSpawnParams.ObjectFlags |= RF_Transient;
	ActorSpawnParams.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ActorSpawnParams.bDeferConstruction = LoadedPawnData->PawnController != nullptr;
	
	APawn* NewPawn = World->SpawnActor<APawn>(LoadedPawnData->PawnData->PawnClass, Location, Rotation, ActorSpawnParams);
	if (NewPawn && LoadedPawnData->PawnController)
	{
		NewPawn->AIControllerClass = LoadedPawnData->PawnController;
		if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
		{
			PawnExtComp->SetPawnData(LoadedPawnData->PawnData);
		}
		NewPawn->FinishSpawning(FTransform(Rotation, Location, FVector::OneVector));
	}
	
	if (NewPawn && !NewPawn->Controller)
	{
		NewPawn->SpawnDefaultController();
	}

	return NewPawn;
}

void UCruorRTS_UnitSpawnerComponent::SpawnMassUnits(const TSoftObjectPtr<UCruorRTS_MassUnitData> MassUnitData)
{
	if (!IsValid(SpawnDataGenerator.GeneratorInstance))
	{
		UE_VLOG_UELOG(this, LogCruorRTS, Warning, TEXT("No Spawn Data Generators configured."));
		return;
	}

	if (SpawnDataGenerator.Proportion <= 0.0f)
	{
		UE_VLOG_UELOG(this, LogCruorRTS, Error, TEXT("SpawnDataGenerator Proportion must be greater than 0."));
		return;
	}
	
	if (StreamingHandle.IsValid() && StreamingHandle->IsActive())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}

	const UCruorRTS_MassUnitData* LoadedMassData = MassUnitData.Get();
	if (!LoadedMassData)
	{
		return;
	}

	int32 TotalSpawnCount = 0;
	for (const FMassSpawnedEntityType& Unit : LoadedMassData->MassUnits)
	{
		TotalSpawnCount += Unit.Proportion;
	}

	FFinishedGeneratingSpawnDataSignature Delegate = FFinishedGeneratingSpawnDataSignature::CreateUObject(this, &UCruorRTS_UnitSpawnerComponent::OnMassUnitsSpawnDataGenerationFinished, LoadedMassData->MassUnits, GetOwner());
	SpawnDataGenerator.GeneratorInstance->Generate( *GetOwner(), LoadedMassData->MassUnits, TotalSpawnCount, Delegate);	
}

void UCruorRTS_UnitSpawnerComponent::DoDespawning()
{
	if (AllSpawnedEntities.IsEmpty())
	{
		return;
	}
	
	UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!SpawnerSystem)
	{
		UE_LOG(LogCruorRTS, Error, TEXT("UMassSpawnerSubsystem missing while trying to despawn entities"));
		return;
	}

	for (const auto& [TemplateID, Entities] : AllSpawnedEntities)
	{
		SpawnerSystem->DestroyEntities(Entities);
	}
	AllSpawnedEntities.Reset();

	OnDespawningFinishedEvent.Broadcast();
}

void UCruorRTS_UnitSpawnerComponent::DoDespawning(TConstArrayView<FMassEntityHandle> EntitiesToIgnore)
{
	UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!SpawnerSystem)
	{
		UE_LOG(LogCruorRTS, Error, TEXT("UMassSpawnerSubsystem missing while trying to despawn entities"));
		return;
	}

	TSet<FMassEntityHandle> IgnoreSet;
	IgnoreSet.Append(EntitiesToIgnore);

	TArray<FSpawnedEntities> AllEntitiesToKeep;
	TArray<FMassEntityHandle> EntitiesToDestroy;

	for (auto& [TemplateID, Entities] : AllSpawnedEntities)
	{
		FSpawnedEntities KeptEntities;
		KeptEntities.TemplateID = TemplateID;

		for (const FMassEntityHandle& Entity : Entities)
		{
			if (IgnoreSet.Contains(Entity))
			{
				KeptEntities.Entities.Add(Entity);
			}
			else
			{
				EntitiesToDestroy.Add(Entity);
			}
		}

		if (KeptEntities.Entities.Num() > 0)
		{
			AllEntitiesToKeep.Add(MoveTemp(KeptEntities));
		}
	}

	if (EntitiesToDestroy.Num() > 0)
	{
		SpawnerSystem->DestroyEntities(EntitiesToDestroy);
	}

	AllSpawnedEntities = MoveTemp(AllEntitiesToKeep);
	OnDespawningFinishedEvent.Broadcast();
}

bool UCruorRTS_UnitSpawnerComponent::DespawnEntity(const FMassEntityHandle Entity)
{
	UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!SpawnerSystem)
	{
		return false;
	}

	for (auto& [TemplateID, Entities] : AllSpawnedEntities)
	{
		if (const int32 Index = Entities.Find(Entity); Index != INDEX_NONE)
		{
			SpawnerSystem->DestroyEntities(MakeArrayView(&Entity, 1));
			Entities.RemoveAtSwap(Index, EAllowShrinking::No);
			return true;
		}
	}
	return false;
}

int32 UCruorRTS_UnitSpawnerComponent::GetCount() const
{
	return Count;
}


void UCruorRTS_UnitSpawnerComponent::OnMassUnitsSpawnDataGenerationFinished(
	TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results, TArray<FMassSpawnedEntityType> EntityTypes, AActor* Owner)
{
	SpawnGeneratedMassUnits(Results, EntityTypes);
}

void UCruorRTS_UnitSpawnerComponent::SpawnGeneratedMassUnits(TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results,
	TArray<FMassSpawnedEntityType> EntityTypes)
{
	UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!SpawnerSystem)
	{
		return;
	}

	const UWorld* World = GetWorld();
	check(World);

	int32 TotalNum = 0;
	const int32 StartIndex = AllSpawnedEntities.Num();

	FMassEntityManager& EntityManager = SpawnerSystem->GetEntityManagerChecked();		
	
	constexpr int32 Columns = 3;
	constexpr float Spacing = 150.0f;
	constexpr float HalfWidth = ((Columns - 1) * Spacing) / 2.0f;

	for (const FMassEntitySpawnDataGeneratorResult& Result : Results)
	{
		const FRotator CurrentRotation = FRotator(0.f, 360.f / MassUnitCount * CurrentMassUnitIndex, 0.f);
		const FVector StartLocation = CurrentRotation.Vector() * 1500.f + GetOwner()->GetActorLocation();
		
		if (Result.NumEntities <= 0) continue;
		check(EntityTypes.IsValidIndex(Result.EntityConfigIndex));
		const FMassSpawnedEntityType& EntityType = EntityTypes[Result.EntityConfigIndex];

		if (const UMassEntityConfigAsset* EntityConfig = EntityType.GetEntityConfig())
		{
			if (const FMassEntityTemplate& EntityTemplate = EntityConfig->GetOrCreateEntityTemplate(*World); EntityTemplate.IsValid())
			{	
				// 1. Create the Squad Anchor Entity for this squad
				const FMassArchetypeHandle AnchorArchetype = EntityManager.CreateArchetype({
					FTransformFragment::StaticStruct(),
					FCruorRTS_SquadFragment::StaticStruct(),
					FAgentRadiusFragment::StaticStruct(),
					FMassMoveTargetFragment::StaticStruct(),
					FMassDesiredMovementFragment::StaticStruct(),
					FMassVelocityFragment::StaticStruct(),
					FMassForceFragment::StaticStruct(),
					FMassSteeringFragment::StaticStruct(),
					FMassNavMeshCachedPathFragment::StaticStruct(),
					FMassNavMeshShortPathFragment::StaticStruct()
				});

				const FMassEntityHandle SquadEntity = EntityManager.CreateEntity(AnchorArchetype);

				FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadEntity);
				if (SquadFragment)
				{
					SquadFragment->SquadDestination = StartLocation;
					SquadFragment->SquadRotation = CurrentRotation.Quaternion();
					SquadFragment->FormationType = EFormationType::Rectangle;
					SquadFragment->ActiveMemberCount = Result.NumEntities;
				}

				if (FTransformFragment* AnchorTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(SquadEntity))
				{
					AnchorTransform->SetTransform(FTransform(CurrentRotation, StartLocation));
				}
				
				if (FAgentRadiusFragment* AnchorRadius = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(SquadEntity))
				{
					AnchorRadius->Radius = 500.f;
				}

				// 2. Batch Create Squad Member Entities
				auto& [TemplateID, Entities] = AllSpawnedEntities.AddDefaulted_GetRef();
				TemplateID = EntityTemplate.GetTemplateID();
				
				auto CreationContext = EntityManager.BatchCreateEntities(EntityTemplate.GetArchetype(), EntityTemplate.GetSharedFragmentValues(), Result.NumEntities, Entities);
							
				TArray<FInstancedStruct> FragmentInstances;
				FCruorRTS_SquadMemberFragment SquadMemberFragment;
				SquadMemberFragment.SquadEntity = SquadEntity;
				FragmentInstances.Append(EntityTemplate.GetInitialFragmentValues());
				FragmentInstances.Add(FInstancedStruct::Make(SquadMemberFragment));

				EntityManager.BatchSetEntityFragmentValues(CreationContext->GetEntityCollections(EntityManager), FragmentInstances);
				
				UCruorRTS_SquadMovementSubsystem* const SquadSubsystem = EntityManager.GetWorld()->GetSubsystem<
					UCruorRTS_SquadMovementSubsystem>();
				ensure(SquadSubsystem);
				
				TArray<FVector> InitialPositions;
				SquadSubsystem->CalculateNewOffsets(*SquadFragment, SquadFragment->FormationType, Entities.Num(), InitialPositions);
				
				for (int32 Index = 0; Index < InitialPositions.Num(); ++Index)
				{
					const FMassEntityHandle Entity = Entities[Index];
					if (FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity))
					{
						TransformFragment->SetTransform(FTransform(CurrentRotation, InitialPositions[Index]));
					}
				}
				
				SquadSubsystem->AddSquad(SquadEntity, Entities);
				SquadSubsystem->UpdateSquadOffsets(SquadEntity);
				SquadSubsystem->SetSquadRotation(SquadEntity, CurrentRotation.Quaternion());
				
				CurrentMassUnitIndex++;
				TotalNum += Entities.Num();
			}
		}
	}

	if (TotalNum)
	{
		TArray<UMassProcessor*> Processors;
		TSet<TSubclassOf<UMassProcessor>> AddedProcessorClasses;

		for (const FMassEntitySpawnDataGeneratorResult& Result : Results)
		{
			for (const TSubclassOf<UMassProcessor>& ProcessorClass : Result.PostSpawnProcessors)
			{
				if (!AddedProcessorClasses.Contains(ProcessorClass))
				{
					if (UMassProcessor* Processor = GetPostSpawnProcessor(ProcessorClass))
					{
						Processors.Add(Processor);
					}
					AddedProcessorClasses.Add(ProcessorClass);
				}
			}
		}

		if (Processors.Num() > 0)
		{
			FMassEntityManager& EntityManagerRef = UE::Mass::Utils::GetEntityManagerChecked(*World);
			FMassProcessingContext ProcessingContext(EntityManagerRef, 0.0f);

			TArray<FMassEntityHandle> AllEntities;
			AllEntities.Reserve(TotalNum);
			for (int32 Index = StartIndex; Index < AllSpawnedEntities.Num(); ++Index)
			{
				AllEntities.Append(AllSpawnedEntities[Index].Entities);
			}

			TArray<FMassArchetypeEntityCollection> EntityCollections;
			UE::Mass::Utils::CreateEntityCollections(EntityManagerRef, AllEntities, FMassArchetypeEntityCollection::NoDuplicates, EntityCollections);
			UE::Mass::Executor::RunProcessorsView(Processors, ProcessingContext, EntityCollections);
		}
	}
	
	CruorRTS_UnitSpawnerOnSpawningFinishedEvent.Broadcast();
}

UMassProcessor* UCruorRTS_UnitSpawnerComponent::GetPostSpawnProcessor(TSubclassOf<UMassProcessor> ProcessorClass)
{
	if (!ProcessorClass) return nullptr;

	TObjectPtr<UMassProcessor>* const Found = PostSpawnProcessors.FindByPredicate([ProcessorClass](const UMassProcessor* MassProcessor)
	{
		return MassProcessor && MassProcessor->GetClass() == ProcessorClass;
	});

	if (Found) return *Found;

	if (FMassEntityManager* EntityManager = UE::Mass::Utils::GetEntityManager(this))
	{
		UMassProcessor* NewProcessor = NewObject<UMassProcessor>(this, ProcessorClass);
		NewProcessor->CallInitialize(this, EntityManager->AsShared());
		PostSpawnProcessors.Add(NewProcessor);
		return NewProcessor;
	}

	return nullptr;
}
