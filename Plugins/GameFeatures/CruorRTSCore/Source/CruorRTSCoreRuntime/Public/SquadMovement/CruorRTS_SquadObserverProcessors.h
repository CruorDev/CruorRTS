//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "CruorRTS_SquadObserverProcessors.generated.h"

/**
 * Observer for Squad initialization.
 */
UCLASS(MinimalAPI)
class UCruorRTS_SquadInitializeObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:
	CRUORRTSCORERUNTIME_API explicit UCruorRTS_SquadInitializeObserver();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};

/**
 * Observer for Squad member removal / death (compacts ranks & cleans up anchor if empty).
 */
UCLASS(MinimalAPI)
class UCruorRTS_SquadMemberDeathObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:
	CRUORRTSCORERUNTIME_API explicit UCruorRTS_SquadMemberDeathObserver();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};


