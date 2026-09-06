//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CruorRTS_SquadProcessors.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTSSquadMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	
	explicit UCruorRTSSquadMovementProcessor(const FObjectInitializer& ObjectInitializer);
	
protected:
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	
	void UpdateShortPaths(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	
private:
	
	FMassEntityQuery FormationQuery;
};
