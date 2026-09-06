//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#include "SquadMovement/CruorRTS_SquadSignalProcessors.h"

#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "NavCorridor.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "CruorRTS/CruorRTSLogChannels.h"
#include "MassNavigationSubsystem.h"

UCruorRTSFormationUpdated::UCruorRTSFormationUpdated()
{
	bRequiresGameThreadExecution = true;
}

void UCruorRTSFormationUpdated::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	const auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, CruorRTS::MassSquadMovement::Signals::FormationUpdated);
}

void UCruorRTSFormationUpdated::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshCachedPathFragment>(EMassFragmentAccess::ReadWrite);	
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadWrite);	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentHeightFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FCruorRTS_SquadMemberMovementSettingsFragment>();
}

void UCruorRTSFormationUpdated::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [&EntityManager](FMassExecutionContext& ChunkContext)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = ChunkContext.GetMutableFragmentView<FMassMoveTargetFragment>();
		TArrayView<FMassDesiredMovementFragment> DesiredMovementFragments = ChunkContext.GetMutableFragmentView<FMassDesiredMovementFragment>();
		TArrayView<FMassNavMeshCachedPathFragment> CachedPathFragments = ChunkContext.GetMutableFragmentView<FMassNavMeshCachedPathFragment>();
		TArrayView<FMassNavMeshShortPathFragment> ShortPathFragments = ChunkContext.GetMutableFragmentView<FMassNavMeshShortPathFragment>();
		TArrayView<FTransformFragment> TransformFragments = ChunkContext.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentHeightFragment> AgentHeightFragments = ChunkContext.GetFragmentView<FAgentHeightFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusFragments = ChunkContext.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FCruorRTS_SquadMemberFragment> SquadMemberFragments = ChunkContext.GetFragmentView<FCruorRTS_SquadMemberFragment>();
		
		const FCruorRTS_SquadMemberMovementSettingsFragment& FormationSettings = ChunkContext.GetConstSharedFragment<FCruorRTS_SquadMemberMovementSettingsFragment>();
		const FMassMovementParameters& MovementParameters = ChunkContext.GetConstSharedFragment<FMassMovementParameters>();
		
		for (int32 EntityIndex = 0; EntityIndex < ChunkContext.GetNumEntities(); ++EntityIndex)
		{
			const FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadMemberFragments[EntityIndex].SquadEntity);
			if (!SquadFragment) continue;
			FTransformFragment& TransformFragment = TransformFragments[EntityIndex];
			TransformFragment.SetTransform(FTransform(SquadFragment->SquadRotation, TransformFragment.GetTransform().GetLocation(), TransformFragment.GetTransform().GetScale3D()));
			
			const FMassEntityHandle CurrentEntity = ChunkContext.GetEntity(EntityIndex);

			const FNavAgentProperties NavAgentProperties = FNavAgentProperties(
				AgentRadiusFragments[EntityIndex].Radius, AgentHeightFragments[EntityIndex].Height);
			UNavigationSystemV1* NavMeshSubsystem = Cast<UNavigationSystemV1>(ChunkContext.GetWorld()->GetNavigationSystem());
			if (!NavMeshSubsystem)
			{
				UE_LOG(LogCruorRTS, Warning, TEXT("UCruorRTSFormationUpdated: Missing navigation system."));
				continue;
			}
			const FVector& AgentLocation = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const ANavigationData* NavData = NavMeshSubsystem->GetNavDataForProps(NavAgentProperties, AgentLocation);
			if (!NavData)
			{
				UE_LOG(LogCruorRTS, Warning, TEXT("UCruorRTSFormationUpdated: Invalid NavData"));
				continue;
			}
			const FVector TargetLocation = SquadFragment->SquadDestination + SquadMemberFragments[EntityIndex].Offset;
			FPathFindingQuery Query(NavMeshSubsystem, *NavData, AgentLocation, TargetLocation);
			Query.SetAllowPartialPaths(true);
			if (!Query.NavData.IsValid())
			{
				Query.NavData = NavMeshSubsystem->GetNavDataForProps(NavAgentProperties, Query.StartLocation);
			}

			FPathFindingResult Result(ENavigationQueryResult::Error);
			if (Query.NavData.IsValid())
			{
				Result = Query.NavData->FindPath(NavAgentProperties, Query);
			}

			if (!Result.IsSuccessful() || !Result.Path.IsValid())
			{
				UE_LOG(LogCruorRTS, Warning,
				       TEXT(
					       "UCruorRTSFormationUpdated: Initial path query failed, searching nearby location using FNavigationObstacleHashGrid2D..."
				       ));

				UMassNavigationSubsystem* MassNavSubsystem = UWorld::GetSubsystem<UMassNavigationSubsystem>(ChunkContext.GetWorld());
				FNavLocation NearbyNavLocation;
				bool bFoundNearbyLocation = false;

				const float SearchRadius = 1000.f;
				const float Step = 150.f;

				TArray<FVector> CandidatePoints;
				for (float dx = -SearchRadius; dx <= SearchRadius; dx += Step)
				{
					for (float dy = -SearchRadius; dy <= SearchRadius; dy += Step)
					{
						const FVector CandidatePoint = TargetLocation + FVector(dx, dy, 0.f);
						CandidatePoints.Add(CandidatePoint);
					}
				}

				// Sort candidate locations by distance to TargetLocation
				CandidatePoints.Sort([&TargetLocation](const FVector& A, const FVector& B)
				{
					return FVector::DistSquared2D(A, TargetLocation) < FVector::DistSquared2D(B, TargetLocation);
				});

				for (const FVector& CandidatePoint : CandidatePoints)
				{
					// Query FNavigationObstacleHashGrid2D if MassNavigationSubsystem is present
					if (MassNavSubsystem)
					{
						const FNavigationObstacleHashGrid2D& ObstacleGrid = MassNavSubsystem->GetObstacleGrid();
						const float AgentRadius = AgentRadiusFragments[EntityIndex].Radius;
						const FBox CandidateBounds(CandidatePoint - FVector(AgentRadius, AgentRadius, 100.f),
						                           CandidatePoint + FVector(AgentRadius, AgentRadius, 100.f));

						TArray<FMassNavigationObstacleItem> OverlappingObstacles;
						ObstacleGrid.QuerySmall(CandidateBounds, OverlappingObstacles);

						if (OverlappingObstacles.Num() > 0)
						{
							continue; // Skip candidate location if it overlaps obstacles in the hash grid
						}
					}

					// Project candidate point to navigation mesh
					if (NavMeshSubsystem->ProjectPointToNavigation(CandidatePoint, NearbyNavLocation,
					                                               FVector(Step, Step, 500.f), NavData))
					{
						FPathFindingQuery NewQuery(NavMeshSubsystem, *NavData, AgentLocation,
						                           NearbyNavLocation.Location);
						NewQuery.SetAllowPartialPaths(true);
						if (!NewQuery.NavData.IsValid())
						{
							NewQuery.NavData = NavMeshSubsystem->GetNavDataForProps(
								NavAgentProperties, NewQuery.StartLocation);
						}

						if (NewQuery.NavData.IsValid())
						{
							FPathFindingResult NewResult = NewQuery.NavData->FindPath(NavAgentProperties, NewQuery);
							if (NewResult.IsSuccessful() && NewResult.Path.IsValid() && NewResult.Path->GetPathPoints().
								Num() >= 1)
							{
								Result = NewResult;
								bFoundNearbyLocation = true;
								break;
							}
						}
					}
				}

				if (!bFoundNearbyLocation || !Result.IsSuccessful() || !Result.Path.IsValid())
				{
					UE_LOG(LogCruorRTS, Warning,
					       TEXT(
						       "UCruorRTSFormationUpdated: No path found after FNavigationObstacleHashGrid2D nearby location search."
					       ));
					continue;
				}
			}

			Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);

			if (Result.Path.Get()->GetPathPoints().Num() < 1)
			{
				UE_LOG(LogCruorRTS, Warning, TEXT("UCruorRTSFormationUpdated: Path Points < 1"));
			}

			FMassNavMeshCachedPathFragment& CachedPathFragment = CachedPathFragments[EntityIndex];
			CachedPathFragment.NavPath = Result.Path;
			CachedPathFragment.PathSource = EMassNavigationPathSource::NavMesh;
			CachedPathFragment.Corridor = MakeShared<FNavCorridor>();
			const FSharedConstNavQueryFilter NavQueryFilter = Query.QueryFilter
				                                                  ? Query.QueryFilter
				                                                  : NavData->GetDefaultQueryFilter();

			FNavCorridorParams CorridorParams;
			CorridorParams.SetFromWidth(300.f); //@TODO: Change this to configurable property
			CorridorParams.PathOffsetFromBoundaries = 150.f; //@TODO: Change this to configurable property

			UE_LOG(LogCruorRTS, Verbose, TEXT("UCruorRTSFormationUpdated: "));
			UE_LOG(LogCruorRTS, Verbose, TEXT("Start: %s, End: %s"),
			       *CachedPathFragment.NavPath->GetPathPoints()[0].Location.ToCompactString(),
			       *CachedPathFragment.NavPath->GetPathPoints().Last().Location.ToCompactString());
			UE_LOG(LogCruorRTS, Verbose, TEXT("Corridor params - %s"), *CorridorParams.ToString());

			CachedPathFragment.Corridor->BuildFromPath(*CachedPathFragment.NavPath, NavQueryFilter, CorridorParams);

			if (CorridorParams.PathOffsetFromBoundaries > 50.f)
			{
				CachedPathFragment.Corridor->OffsetPathLocationsFromWalls(CorridorParams.PathOffsetFromBoundaries);
			}

			FMassNavMeshShortPathFragment& ShortPathFragment = ShortPathFragments[EntityIndex];
			ShortPathFragment.Reset();
			ShortPathFragment.RequestShortPath(CachedPathFragment.Corridor, /*NavCorridorStartIndex*/0,
			                                   /*NumLeadingPoints*/0, /*@TODO: Change this to configurable property*/
			                                   20.f);

			CachedPathFragment.NavPathNextStartIndex = (uint16)FMath::Max(
				ShortPathFragment.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate -
				FMassNavMeshCachedPathFragment::NumLeadingPoints, 0);

			float DesiredSpeed = FMath::Min(
				MovementParameters.GenerateDesiredSpeed(FormationSettings.RunMovement, CurrentEntity.Index) *
				/*@TODO: Change this to configurable property*/1.f, MovementParameters.MaxSpeed);
			DesiredSpeed = FMath::Min(DesiredSpeed, DesiredMovementFragments[EntityIndex].DesiredMaxSpeedOverride);

			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FCruorRTS_SquadMemberFragment& SquadMemberFragment = SquadMemberFragments[EntityIndex];

			MoveTarget.Center = SquadFragment->SquadDestination + SquadMemberFragment.Offset;
			MoveTarget.Forward = SquadFragment->SquadRotation.Vector();
			MoveTarget.DistanceToGoal = (AgentLocation - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 50.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
			MoveTarget.DesiredSpeed.Set(DesiredSpeed);
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *ChunkContext.GetWorld());
		}
	});
}
