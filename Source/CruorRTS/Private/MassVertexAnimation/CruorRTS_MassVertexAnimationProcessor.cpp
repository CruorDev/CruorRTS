//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "MassVertexAnimation/CruorRTS_MassVertexAnimationProcessor.h"

#include "AnimToTextureInstancePlaybackHelpers.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassVertexAnimation/CruorRTS_MassVertexAnimationFragments.h"

UCruorRTS_MassVertexAnimationProcessor::UCruorRTS_MassVertexAnimationProcessor() : EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorldToMass);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);

	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
}

void UCruorRTS_MassVertexAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{	
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCruorRTS_MassVertexAnimationInfoFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FCruorRTS_MassVertexAnimationParametersFragment>();
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);	
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
}

void UCruorRTS_MassVertexAnimationProcessor::InitializeInternal(UObject& Owner,
                                                                const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	
	World = Owner.GetWorld();
	check(World);
}

void UCruorRTS_MassVertexAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UpdateVertexAnimation)
		UMassRepresentationSubsystem* RepresentationSubsystem = ChunkContext.GetMutableSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);		
		
		const FCruorRTS_MassVertexAnimationParametersFragment& VertexAnimationParametersFragment = ChunkContext.GetConstSharedFragment<FCruorRTS_MassVertexAnimationParametersFragment>();
		const UAnimToTextureDataAsset* AnimToTextureDataAsset = VertexAnimationParametersFragment.AnimToTextureData.Get();
		if (!AnimToTextureDataAsset) return;
		
		const TConstArrayView<FMassVelocityFragment> MassVelocityFragments = ChunkContext.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FMassRepresentationFragment> RepresentationFragments = ChunkContext.GetFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODFragments = ChunkContext.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FCruorRTS_MassVertexAnimationInfoFragment> VertexAnimationInfoFragments = ChunkContext.GetMutableFragmentView<FCruorRTS_MassVertexAnimationInfoFragment>();
		
		const float GlobalTime = World->GetTimeSeconds();		
		
		for (FMassExecutionContext::FEntityIterator EntityIt = ChunkContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassRepresentationFragment& RepresentationFragment = RepresentationFragments[EntityIt];
				
			if (RepresentationFragment.CurrentRepresentation != EMassRepresentationType::StaticMeshInstance ||
				!RepresentationFragment.StaticMeshDescHandle.IsValid())
			{
				continue;
			}
			
			const FMassVelocityFragment& MassVelocityFragment = MassVelocityFragments[EntityIt];
			const FMassRepresentationLODFragment& RepresentationLODFragment = RepresentationLODFragments[EntityIt];
			FCruorRTS_MassVertexAnimationInfoFragment& VertexAnimationInfoFragment = VertexAnimationInfoFragments[EntityIt];
			
			const auto StaticMeshDescHandleIndex = RepresentationFragment.StaticMeshDescHandle.ToIndex();
			auto& InstancedStaticMeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos()[StaticMeshDescHandleIndex];
			
			const float PrevPlayRate = VertexAnimationInfoFragment.PlayRate;
			VertexAnimationInfoFragment.GlobalStartTime = 
				GlobalTime - 
					PrevPlayRate * 
						(GlobalTime - VertexAnimationInfoFragment.GlobalStartTime) / 
							VertexAnimationInfoFragment.PlayRate;
			const float Speed = MassVelocityFragment.Value.Length();
			VertexAnimationInfoFragment.AnimationStateIndex = Speed >= 50.f ? VertexAnimationParametersFragment.RunAnimationIndex : VertexAnimationParametersFragment.WalkAnimationIndex;
			if (Speed <= SMALL_NUMBER) VertexAnimationInfoFragment.AnimationStateIndex = VertexAnimationParametersFragment.IdleAnimationIndex;
			
			FAnimToTextureAutoPlayData PlayData; 
			if (UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(
				AnimToTextureDataAsset, 
				VertexAnimationInfoFragment.AnimationStateIndex, 
				PlayData, 
				VertexAnimationInfoFragment.GlobalStartTime, 
				VertexAnimationInfoFragment.PlayRate))
			{
				InstancedStaticMeshInfo.AddBatchedCustomData<FAnimToTextureAutoPlayData>(
					PlayData, 
					RepresentationLODFragment.LOD,
					RepresentationLODFragment.PrevLOD,
					0);
			}
		}
	});
}

void UCruorRTS_MassVertexAnimationProcessor::UpdateVertexAnimation(
	FMassInstancedStaticMeshInfo& InstancedStaticMeshInfo,
	const FCruorRTS_MassVertexAnimationInfoFragment& AnimationData, 
	const float LODSignificance, 
	const float PreviousLODSignificance,
	const int32 NumberOfFloatsToPad /*= 0*/) const
{
	
}
