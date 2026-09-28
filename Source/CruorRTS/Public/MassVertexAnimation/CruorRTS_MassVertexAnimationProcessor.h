//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_MassVertexAnimationFragments.h"
#include "MassRepresentationTypes.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"
#include "CruorRTS_MassVertexAnimationProcessor.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTS_API UCruorRTS_MassVertexAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UCruorRTS_MassVertexAnimationProcessor();
	
protected:
	/** Configure the owned FMassEntityQuery instances to express processor's requirements */
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	UPROPERTY(Transient)
	TObjectPtr<UWorld> World;
	
	FMassEntityQuery EntityQuery;
	
private:
	void UpdateVertexAnimation(
		FMassInstancedStaticMeshInfo& InstancedStaticMeshInfo,
		const FCruorRTS_MassVertexAnimationInfoFragment& AnimationData, 
		float LODSignificance, 
		float PreviousLODSignificance, 
		int32 NumberOfFloatsToPad) const;
};
