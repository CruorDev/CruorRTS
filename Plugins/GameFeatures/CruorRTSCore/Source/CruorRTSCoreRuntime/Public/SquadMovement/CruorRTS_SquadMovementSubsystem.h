//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CruorRTS_SquadFragments.h"
#include "MassEntityQuery.h"
#include "MassSubsystemBase.h"
#include "CruorRTS_SquadMovementSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTS_SquadMovementSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()	
	
public:	
	
	UFUNCTION(BlueprintCallable)
	TArray<FMassEntityHandle> GetSquadMembers(const FMassEntityHandle& SquadHandle) const;
	
	UFUNCTION(BlueprintCallable)
	const TArray<FMassEntityHandle> GetAllSquads() const;
	
	UFUNCTION(BlueprintCallable)
	void AddSquad(const FMassEntityHandle SquadHandle, const TArray<FMassEntityHandle> MemberHandles);
	
	UFUNCTION(BlueprintCallable)
	void RemoveSquadMember(const FMassEntityHandle SquadHandle, const FMassEntityHandle& MemberHandle);
	
	UFUNCTION(BlueprintCallable)
	void SetSquadMoveTarget(const FMassEntityHandle& SquadHandle, const FVector& NewMoveTarget);
	
	UFUNCTION(BlueprintCallable)
	void SetSquadRotation(const FMassEntityHandle& SquadHandle, const FQuat& NewRotation);
	
	UFUNCTION(BlueprintCallable)
	void SetSquadFormation(const FMassEntityHandle& SquadHandle, const EFormationType FormationType);
	
	void UpdateSquadOffsets(const FMassEntityHandle& SquadHandle) const;
	
	static void CreateQueryForSquad(const FMassEntityHandle& SquadHandle, FMassEntityQuery& EntityQuery);
	
	static void CalculateNewOffsets(FCruorRTS_SquadFragment& SquadFragment, const EFormationType FormationType, int Count, TArray<FVector>& OutNewOffsets);

private:

	/**
	 * Solves the linear assignment problem using the Hungarian algorithm (Jonker-Volgenant variant).
	 * Finds the minimum-cost one-to-one matching between N workers and N jobs.
	 *
	 * @param N           - Number of workers/jobs (square matrix dimension).
	 * @param CostMatrix  - Row-major N×N cost matrix; CostMatrix[i*N + j] is the cost of assigning worker i to job j.
	 * @param OutAssignment - Output array of size N; OutAssignment[worker] = job index assigned to that worker.
	 */
	static void HungarianAssignment(int32 N, const TArray<double>& CostMatrix, TArray<int32>& OutAssignment);
	
private:
	
	TMap<FMassEntityHandle, TArray<FMassEntityHandle>> Squads;
};
