// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CruorRTS_MassSelectorPositionProcessor.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTS_MassSelectorPositionProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	
	explicit UCruorRTS_MassSelectorPositionProcessor(const FObjectInitializer& ObjectInitializer);
	
protected:
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	
	FMassEntityQuery EntityQuery;
};
