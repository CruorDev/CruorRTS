// Copyright Cruor enf., All Rights Reserved.


#include "Interaction/Mass/CruorRTS_SelectorQueryProcessor.h"

#include "MassExecutionContext.h"
#include "MassNavigationSubsystem.h"
#include "Interaction/Mass/CruorRTS_InteractionFragments.h"
#include "Interaction/Mass/CruorRTS_MassSelectorSubsystem.h"
#include "CruorRTS/CruorRTSLogChannels.h"
#include "Mass/EntityFragments.h"

UCruorRTS_SelectorQueryProcessor::UCruorRTS_SelectorQueryProcessor(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer), EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	
	bAutoRegisterWithProcessingPhases = true;
}

void UCruorRTS_SelectorQueryProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCruorRTS_InteractionFragment>(EMassFragmentAccess::ReadOnly);
}

void UCruorRTS_SelectorQueryProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UCruorRTS_MassSelectorSubsystem* MassSelectorSubsystem = UWorld::GetSubsystem<UCruorRTS_MassSelectorSubsystem>(EntityManager.GetWorld());
	if (!MassSelectorSubsystem) return;
	
	UMassNavigationSubsystem* MassNavigationSubsystem = UWorld::GetSubsystem<UMassNavigationSubsystem>(EntityManager.GetWorld());
	if (!MassNavigationSubsystem) return;

	const FVector SelectorPos = MassSelectorSubsystem->GetCachedLocalPlayerPosition();
	constexpr float Radius = 10.0f; // @TODO: Make this configurable?
	constexpr float RadiusSq = Radius * Radius;

	const FVector BoxMin(SelectorPos.X - Radius, SelectorPos.Y - Radius, -100.0f);
	const FVector BoxMax(SelectorPos.X + Radius, SelectorPos.Y + Radius, 100.0f);
	const FBox QueryBounds(BoxMin, BoxMax);

	TArray<FMassNavigationObstacleItem> QueriedEntities;
	MassNavigationSubsystem->GetObstacleGrid().Query(QueryBounds, QueriedEntities);
	if (QueriedEntities.IsEmpty())
	{
		MassSelectorSubsystem->ResetCurrentOptions();		
		return;
	}
	
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FCruorRTS_SquadMemberFragment> SquadMembers = ChunkContext.GetFragmentView<FCruorRTS_SquadMemberFragment>();
		const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FCruorRTS_InteractionFragment> Interactions = ChunkContext.GetFragmentView<FCruorRTS_InteractionFragment>();		
		
		for (FMassExecutionContext::FEntityIterator EntityIt = ChunkContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassEntityHandle& CurrentEntity = ChunkContext.GetEntity(EntityIt);

			bool bInQuery = false;
			
			for (auto& [Entity, ItemFlags] : QueriedEntities)
			{
				if (Entity == CurrentEntity)
				{
					bInQuery = true;
					break;
				} 
			}
			
			if (!bInQuery) continue;
			
			const FVector& EntityLocation = Transforms[EntityIt].GetTransform().GetLocation();
			if (FVector::Dist2D(EntityLocation, SelectorPos) > 50.f) // @TODO: Make this configurable?
			{
				continue;
			}
		
			FInteractionOption EntityInteractionOption = Interactions[EntityIt].InteractionOption;
		
			if (MassSelectorSubsystem->GetCurrentOptions().Contains(EntityInteractionOption)) return;
		
			MassSelectorSubsystem->AddCurrentOption(EntityInteractionOption);
			MassSelectorSubsystem->SetCachedSquadHandle(SquadMembers[EntityIt].SquadEntity);
			MassSelectorSubsystem->SetCachedWheelMenuGlyphIconData(Interactions[EntityIt].InnerWheelMenuGlyphIconData, Interactions[EntityIt].OuterWheelMenuGlyphIconData);
		}		
	});
}
