//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#include "SquadMovement/CruorRTS_SquadTraits.h"

#include "MassEntitySubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"

void UCruorRTS_SquadTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	const UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	check(EntitySubsystem);
	
	auto& CruorSquadMemberFormationSettings = EntityManager.GetOrCreateConstSharedFragment<FCruorRTS_SquadMemberMovementSettingsFragment>(MemberMovementSettingsFragment);
	BuildContext.AddFragment<FCruorRTS_SquadMemberFragment>();	
	BuildContext.AddConstSharedFragment(CruorSquadMemberFormationSettings);
}
