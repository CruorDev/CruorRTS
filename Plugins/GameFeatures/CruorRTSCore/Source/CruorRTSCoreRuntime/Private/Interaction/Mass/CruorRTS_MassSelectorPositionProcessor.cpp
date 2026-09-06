// Copyright Cruor enf., All Rights Reserved.


#include "Interaction/Mass/CruorRTS_MassSelectorPositionProcessor.h"

#include "Interaction/Mass/CruorRTS_MassSelectorSubsystem.h"
#include "Mass/EntityFragments.h"

UCruorRTS_MassSelectorPositionProcessor::UCruorRTS_MassSelectorPositionProcessor(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;	
}

void UCruorRTS_MassSelectorPositionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UCruorRTS_MassSelectorPositionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (UCruorRTS_MassSelectorSubsystem* MassSelectorSubsystem = UWorld::GetSubsystem<UCruorRTS_MassSelectorSubsystem>(EntityManager.GetWorld())) 
		MassSelectorSubsystem->UpdateLocalPlayerSelectorPosition(EntityManager.GetWorld());
}
