//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#include "SquadMovement/CruorRTS_SquadProcessors.h"

#include "DrawDebugHelpers.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "NavCorridor.h"
#include "NavigationSystem.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "SquadMovement/CruorRTS_SquadSignalProcessors.h"
#include "CruorRTS/CruorRTSLogChannels.h"
#include "Mass/EntityFragments.h"
#include "Steering/MassSteeringFragments.h"

UCruorRTSSquadMovementProcessor::UCruorRTSSquadMovementProcessor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), FormationQuery()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
}

void UCruorRTSSquadMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FormationQuery.Initialize(EntityManager);
	
	FormationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	FormationQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	FormationQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassSteeringFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassNavMeshCachedPathFragment>(EMassFragmentAccess::ReadWrite);
	FormationQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadWrite);
	
	FormationQuery.RegisterWithProcessor(*this);
}

void UCruorRTSSquadMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UpdateShortPaths(EntityManager, Context);
}

void UCruorRTSSquadMovementProcessor::UpdateShortPaths(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	FormationQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ContextChunk)
	{
		const int32 NumOfEntitiesInChunk = ContextChunk.GetNumEntities();
		if (NumOfEntitiesInChunk == 0) return;
		
		TArrayView<FTransformFragment> Transforms = ContextChunk.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FMassMoveTargetFragment> Targets = ContextChunk.GetMutableFragmentView<FMassMoveTargetFragment>();
		TArrayView<FMassSteeringFragment> Steerings = ContextChunk.GetMutableFragmentView<FMassSteeringFragment>();
		TArrayView<FMassForceFragment> Forces = ContextChunk.GetMutableFragmentView<FMassForceFragment>();
		TArrayView<FMassVelocityFragment> Velocities = ContextChunk.GetMutableFragmentView<FMassVelocityFragment>();
		TArrayView<FMassNavMeshCachedPathFragment> CachedPathFragments = ContextChunk.GetMutableFragmentView<FMassNavMeshCachedPathFragment>();
		TArrayView<FMassNavMeshShortPathFragment> ShortPathFragments = ContextChunk.GetMutableFragmentView<FMassNavMeshShortPathFragment>();
		const TConstArrayView<FCruorRTS_SquadMemberFragment> SquadMemberFragments = ContextChunk.GetFragmentView<FCruorRTS_SquadMemberFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumOfEntitiesInChunk; ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = Targets[EntityIndex];
			FMassSteeringFragment& Steering = Steerings[EntityIndex];
			FMassNavMeshCachedPathFragment& CachedPathFragment = CachedPathFragments[EntityIndex];
			FMassNavMeshShortPathFragment& ShortPathFragment = ShortPathFragments[EntityIndex];
			
			const FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadMemberFragments[EntityIndex].SquadEntity);
			
			if (!CachedPathFragment.NavPath.IsValid() && ShortPathFragment.NumPoints <= 1)
			{
				continue;	
			}
			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Stand) 
			{				
				Forces[EntityIndex].Value = FVector::ZeroVector;
				Velocities[EntityIndex].Value = FVector::ZeroVector;
				Steerings[EntityIndex].Reset();
				Transforms[EntityIndex].SetTransform(FTransform(MoveTarget.Forward.Rotation(), MoveTarget.Center, FVector(1.f)));
				continue;
			}
			if (ShortPathFragment.IsDone() && ShortPathFragment.bPartialResult)
			{
				UE_LOG(LogCruorRTS, Verbose, TEXT("updating short path, starting at index %i"), CachedPathFragment.NavPathNextStartIndex);
    				
				ShortPathFragment.RequestShortPath(
					CachedPathFragment.Corridor, 
					CachedPathFragment.NavPathNextStartIndex, 
					CachedPathFragment.NumLeadingPoints, 
					20.f);
				CachedPathFragment.NavPathNextStartIndex += 
					static_cast<uint16>(FMath::Max(
						ShortPathFragment.NumPoints
						- FMassNavMeshShortPathFragment::NumPointsBeyondUpdate
						- FMassNavMeshCachedPathFragment::NumLeadingPoints,
						0));
				
				continue;
			}
			// Arrival check
			if (ShortPathFragment.IsDone() && MoveTarget.EntityDistanceToGoal <= MoveTarget.SlackRadius)
			{
				CachedPathFragments[EntityIndex].NavPath.Reset();
				ShortPathFragments[EntityIndex].Reset();
				Steering.Reset();
				MoveTarget.Forward = SquadFragment->SquadRotation.Vector();
    				
				MoveTarget.CreateNewAction(EMassMovementAction::Stand, *ContextChunk.GetWorld());
			}
		}
	});
}
