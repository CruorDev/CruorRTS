//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "MassVertexAnimation/CruorRTS_MassVertexAnimationTraits.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"

void UCruorRTS_MassVertexAnimationTraits::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
                                                        const UWorld& World) const
{
	BuildContext.AddFragment<FCruorRTS_MassVertexAnimationInfoFragment>();
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct& MassVertexAnimationSharedStruct = EntityManager.GetOrCreateConstSharedFragment(MassVertexAnimationParameters);
	
	BuildContext.AddConstSharedFragment(MassVertexAnimationSharedStruct);
}
