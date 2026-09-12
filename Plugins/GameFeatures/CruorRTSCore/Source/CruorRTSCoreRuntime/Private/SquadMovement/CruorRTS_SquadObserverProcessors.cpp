//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#include "SquadMovement/CruorRTS_SquadObserverProcessors.h"

#include "MassExecutionContext.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "SquadMovement/CruorRTS_SquadMovementSubsystem.h"
#include "Units/Tags/CruorRTS_SquadMemberTags.h"

UCruorRTS_SquadInitializeObserver::UCruorRTS_SquadInitializeObserver()
	: EntityQuery(*this)
{
	ObservedTypes = { FCruorRTS_SquadMemberFragment::StaticStruct() };
	ObservedOperations = EMassObservedOperationFlags::Add; 
}

void UCruorRTS_SquadInitializeObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCruorRTS_SquadMovementSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCruorRTS_SquadInitializeObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TSet<FMassEntityHandle> SquadHandles;
	
	EntityQuery.ForEachEntityChunk(Context, [&SquadHandles](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FCruorRTS_SquadMemberFragment> Members = ChunkContext.GetFragmentView<FCruorRTS_SquadMemberFragment>();
		for (const FCruorRTS_SquadMemberFragment& Member : Members)
		{
			if (Member.SquadEntity.IsValid())
			{
				SquadHandles.Add(Member.SquadEntity);
			}
		}
	});

	const auto SquadMovementSubsystem = UWorld::GetSubsystem<UCruorRTS_SquadMovementSubsystem>(EntityManager.GetWorld());
	if (!SquadMovementSubsystem) return;
	
	for (const FMassEntityHandle& SquadHandle : SquadHandles)
	{
		SquadMovementSubsystem->UpdateSquadOffsets(SquadHandle);
	}
}

UCruorRTS_SquadMemberDeathObserver::UCruorRTS_SquadMemberDeathObserver()
	: EntityQuery(*this)
{
	ObservedTypes = { FDeadTag::StaticStruct() };
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void UCruorRTS_SquadMemberDeathObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCruorRTS_SquadMovementSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCruorRTS_SquadMemberDeathObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TSet<FMassEntityHandle> SquadHandlesToUpdate;
	
	UCruorRTS_SquadMovementSubsystem* const SquadMovementSubsystem = UWorld::GetSubsystem<
		UCruorRTS_SquadMovementSubsystem>(EntityManager.GetWorld());
	if (!SquadMovementSubsystem) return;
	
	const TConstArrayView<FCruorRTS_SquadMemberFragment> Members = Context.GetFragmentView<FCruorRTS_SquadMemberFragment>();	

	EntityQuery.ForEachEntityChunk(Context, [&Members ,&SquadHandlesToUpdate, &EntityManager, &SquadMovementSubsystem](FMassExecutionContext& ChunkContext)
	{
		for (FMassExecutionContext::FEntityIterator EntityIt = ChunkContext.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassEntityHandle& MemberHandle = ChunkContext.GetEntity(EntityIt);
			const FCruorRTS_SquadMemberFragment& MemberFragment = Members[EntityIt];
			if (EntityManager.IsEntityValid(MemberHandle) && EntityManager.IsEntityValid(MemberFragment.SquadEntity))
			{
				FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(MemberFragment.SquadEntity);
				SquadFragment->ActiveMemberCount -= 1;
				SquadHandlesToUpdate.Add(MemberFragment.SquadEntity);
				SquadMovementSubsystem->RemoveSquadMember(MemberFragment.SquadEntity, MemberHandle);				
			}
		}
	});	

	for (const FMassEntityHandle& Handle : SquadHandlesToUpdate)
	{
		if (!EntityManager.IsEntityValid(Handle)) continue;
		
		FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(Handle);
		if (!SquadFragment) continue;

		// If no living squad members remain, destroy the squad entity
		if (SquadFragment->ActiveMemberCount <= 0)
		{
			EntityManager.DestroyEntity(Handle);
			continue;
		}
		
		// Recompute active offsets and update member count
		SquadMovementSubsystem->UpdateSquadOffsets(Handle);
	}
}
