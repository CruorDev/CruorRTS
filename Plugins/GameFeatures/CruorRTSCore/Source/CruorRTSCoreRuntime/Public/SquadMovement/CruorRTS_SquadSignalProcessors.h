//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalProcessorBase.h"
#include "CruorRTS_SquadSignalProcessors.generated.h"

namespace CruorRTS::MassSquadMovement::Signals
{
	const FName FormationUpdated = FName("FormationUpdated");
}

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTSFormationUpdated : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
public:
	
	UCruorRTSFormationUpdated();
	
protected:
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
};
