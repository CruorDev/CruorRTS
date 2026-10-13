//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#include "SquadMovement/CruorRTS_SquadMovementSubsystem.h"

#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "Interaction/Mass/CruorRTS_MassSelectorSubsystem.h"
#include "SquadMovement/CruorRTS_SquadSignalProcessors.h"
#include "Mass/EntityFragments.h"
#include "UObject/FastReferenceCollector.h"


TArray<FMassEntityHandle> UCruorRTS_SquadMovementSubsystem::GetSquadMembers(
	const FMassEntityHandle& SquadHandle) const
{
	return Squads.FindChecked(SquadHandle);
}

const TArray<FMassEntityHandle> UCruorRTS_SquadMovementSubsystem::GetAllSquads() const
{
	TArray<FMassEntityHandle> SquadHandles;
	for (const auto& [SquadEntity, SquadMembers] : Squads)
	{
		SquadHandles.Emplace(SquadEntity);
	}
	return SquadHandles;
}

void UCruorRTS_SquadMovementSubsystem::AddSquad(const FMassEntityHandle SquadHandle,
	const TArray<FMassEntityHandle> MemberHandles)
{
	Squads.FindOrAdd(SquadHandle, MemberHandles);
}

void UCruorRTS_SquadMovementSubsystem::RemoveSquadMember(const FMassEntityHandle SquadHandle,
	const FMassEntityHandle& MemberHandle)
{
	Squads.Find(SquadHandle)->RemoveSingleSwap(MemberHandle);
}

void UCruorRTS_SquadMovementSubsystem::SetSquadMoveTarget(const FMassEntityHandle& SquadHandle, const FVector& NewMoveTarget) 
{
	const FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	if (!EntityManager.IsEntityValid(SquadHandle)) return;

	FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadHandle);
	if (!SquadFragment) return;

	if (const UCruorRTS_MassSelectorSubsystem* MassSelectorSubsystem 
		= UWorld::GetSubsystem<UCruorRTS_MassSelectorSubsystem>(GetWorld()))
	{
		const FVector CachedLocation = MassSelectorSubsystem->GetCachedEntityLocation();
		const FVector Offset = NewMoveTarget - CachedLocation;
		SquadFragment->SquadDestination += Offset;
	}

	if (UMassSignalSubsystem* const SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(
		EntityManager.GetWorld()))
	{
		SignalSubsystem->SignalEntities(CruorRTS::MassSquadMovement::Signals::FormationUpdated, GetSquadMembers(SquadHandle));
	}
}

void UCruorRTS_SquadMovementSubsystem::SetSquadRotation(const FMassEntityHandle& SquadHandle, const FQuat& NewRotation) 
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	if (!EntityManager.IsEntityValid(SquadHandle)) return;

	FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadHandle);
	if (!SquadFragment) return;

	SquadFragment->SquadRotation = NewRotation;
	
	UpdateSquadOffsets(SquadHandle);

	if (UMassSignalSubsystem* const SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(EntityManager.GetWorld()))
	{
		SignalSubsystem->SignalEntities(CruorRTS::MassSquadMovement::Signals::FormationUpdated, GetSquadMembers(SquadHandle));
	}
}

void UCruorRTS_SquadMovementSubsystem::SetSquadFormation(const FMassEntityHandle& SquadHandle,
	const EFormationType FormationType)
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	if (!EntityManager.IsEntityValid(SquadHandle)) return;

	FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadHandle);
	if (!SquadFragment) return;
	
	FMassEntityQuery EntityQuery(EntityManager.AsShared());
	CreateQueryForSquad(SquadHandle, EntityQuery);

	SquadFragment->FormationType = FormationType;
	UpdateSquadOffsets(SquadHandle);

	if (UMassSignalSubsystem* const SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(EntityManager.GetWorld()))
	{
		SignalSubsystem->SignalEntities(CruorRTS::MassSquadMovement::Signals::FormationUpdated, GetSquadMembers(SquadHandle));
	}
}

void UCruorRTS_SquadMovementSubsystem::UpdateSquadOffsets(const FMassEntityHandle& SquadHandle) const
{
	const FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	if (!EntityManager.IsEntityValid(SquadHandle)) return;
	FCruorRTS_SquadFragment* SquadFragment = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadFragment>(SquadHandle);
	// --- Phase 1: Collect current member handles and their existing offsets ---

	TArray<FMassEntityHandle> Members = GetSquadMembers(SquadHandle);
	TArray<FVector> CurrentOffsets;
	
	for (int32 Index = 0; Index < Members.Num(); ++Index)
	{
		CurrentOffsets.Emplace(EntityManager.GetFragmentDataChecked<FCruorRTS_SquadMemberFragment>(Members[Index]).Offset);
	}

	const int32 NumMembers = Members.Num();
	if (NumMembers == 0)
	{
		return;
	}

	// --- Phase 2: Compute the desired formation offsets and rotate them to face the squad's target direction ---

	TArray<FVector> DesiredOffsets;
	CalculateNewOffsets(*SquadFragment, SquadFragment->FormationType,NumMembers, DesiredOffsets);

	if (DesiredOffsets.Num() != NumMembers)
	{
		return;
	}

	const FQuat& SquadRotation = SquadFragment->SquadRotation;
	for (FVector& Offset : DesiredOffsets)
	{
		Offset = SquadRotation.RotateVector(Offset);
	}

	// --- Phase 3: Build the cost matrix (squared 2D distance from each member's current offset to each desired slot) ---

	TArray<double> CostMatrix;
	CostMatrix.SetNumUninitialized(NumMembers * NumMembers);
	for (int32 MemberIdx = 0; MemberIdx < NumMembers; ++MemberIdx)
	{
		for (int32 SlotIdx = 0; SlotIdx < NumMembers; ++SlotIdx)
		{
			CostMatrix[MemberIdx * NumMembers + SlotIdx] = FVector::DistSquared2D(CurrentOffsets[MemberIdx], DesiredOffsets[SlotIdx]);
		}
	}

	// --- Phase 4: Solve the assignment problem to find the minimum-cost member → slot pairing ---

	TArray<int32> MemberToSlot;
	HungarianAssignment(NumMembers, CostMatrix, MemberToSlot);

	// --- Phase 5: Apply the assigned offsets and elect the squad leader (the member at the zero-offset anchor) ---

	int32 GlobalMemberIndex = 0;
	for (int32 MemberIdx = 0; MemberIdx < NumMembers; ++MemberIdx)
	{		
		FCruorRTS_SquadMemberFragment* SquadMember = EntityManager.GetFragmentDataPtr<FCruorRTS_SquadMemberFragment>(Members[MemberIdx]);
		const int32 AssignedSlot = MemberToSlot[GlobalMemberIndex];
		const FVector& AssignedOffset = DesiredOffsets[AssignedSlot];
		SquadMember->Offset = AssignedOffset;		
		++GlobalMemberIndex;
	}
	if (UMassSignalSubsystem* const SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(EntityManager.GetWorld()))
	{	
		SignalSubsystem->SignalEntities(CruorRTS::MassSquadMovement::Signals::FormationUpdated, Members);		
	}
}

void UCruorRTS_SquadMovementSubsystem::HungarianAssignment(const int32 N, const TArray<double>& CostMatrix, TArray<int32>& OutAssignment)
{

	TArray<double> DualRow;  DualRow.Init(0.0, N + 1);
	TArray<double> DualCol;  DualCol.Init(0.0, N + 1);
	TArray<int32>  ColAssignment; ColAssignment.Init(0, N + 1);
	TArray<int32>  AugmentPath;   AugmentPath.Init(0, N + 1);

	// Augment the matching one worker at a time.
	for (int32 Worker = 1; Worker <= N; ++Worker)
	{
		// Set the unassigned-sentinel column to point at the current worker.
		ColAssignment[0] = Worker;

		int32 CurrentCol = 0; // Start the augmenting path from the sentinel column.
		TArray<double> MinReducedCost; MinReducedCost.Init(DBL_MAX, N + 1);
		TArray<bool>   ColVisited;     ColVisited.Init(false, N + 1);

		// Shortest-path search: find the minimum-cost augmenting path from Worker to a free job.
		do
		{
			ColVisited[CurrentCol] = true;
			const int32 CurrentWorker = ColAssignment[CurrentCol];
			double PathImprovement = DBL_MAX;
			int32  BestNextCol = 0;

			for (int32 Col = 1; Col <= N; ++Col)
			{
				if (ColVisited[Col])
				{
					continue;
				}

				// Reduced cost = actual cost − dual variables (always ≥ 0 by LP complementary slackness).

				if (const double ReducedCost 
					= CostMatrix[(CurrentWorker - 1) * N + (Col - 1)] - DualRow[CurrentWorker] - DualCol[Col]; 
					ReducedCost < MinReducedCost[Col])
				{
					MinReducedCost[Col] = ReducedCost;
					AugmentPath[Col] = CurrentCol;
				}

				if (MinReducedCost[Col] < PathImprovement)
				{
					PathImprovement = MinReducedCost[Col];
					BestNextCol = Col;
				}
			}

			// Update dual variables by the amount the path improved (keeps reduced costs non-negative).
			for (int32 Col = 0; Col <= N; ++Col)
			{
				if (ColVisited[Col])
				{
					DualRow[ColAssignment[Col]] += PathImprovement;
					DualCol[Col]               -= PathImprovement;
				}
				else
				{
					MinReducedCost[Col] -= PathImprovement;
				}
			}

			CurrentCol = BestNextCol;
		}
		while (ColAssignment[CurrentCol] != 0); // Stop when we reach a free (unassigned) job column.

		// Augment: walk back along AugmentPath and flip assignments.
		do
		{
			const int32 PredecessorCol = AugmentPath[CurrentCol];
			ColAssignment[CurrentCol]  = ColAssignment[PredecessorCol];
			CurrentCol                 = PredecessorCol;
		}
		while (CurrentCol != 0);
	}

	// Decode ColAssignment (job → worker, 1-indexed) into OutAssignment (worker → job, 0-indexed).
	OutAssignment.SetNumUninitialized(N);
	for (int32 Col = 1; Col <= N; ++Col)
	{
		const int32 AssignedWorker = ColAssignment[Col];
		if (AssignedWorker > 0 && AssignedWorker <= N)
		{
			OutAssignment[AssignedWorker - 1] = Col - 1;
		}
	}
}

void UCruorRTS_SquadMovementSubsystem::CreateQueryForSquad(const FMassEntityHandle& SquadHandle, FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddRequirement<FCruorRTS_SquadMemberFragment>(EMassFragmentAccess::ReadWrite);
}

void UCruorRTS_SquadMovementSubsystem::CalculateNewOffsets(FCruorRTS_SquadFragment& SquadFragment, EFormationType FormationType,
	int Count,
	TArray<FVector>& OutNewOffsets)
{
	if (FormationType == EFormationType::Rectangle)
	{
		OutNewOffsets.Empty();
		const auto& [SquadWidth, BufferDistance] = SquadFragment.FormationSettings;	
	
		const FVector CenterOffset = FVector(Count/SquadWidth/2 * BufferDistance, SquadWidth/2 * BufferDistance, 0.0f);
	
		int PlacedMembers = 0;
		int Index = 0;
		while (PlacedMembers < Count)
		{
			float Width = Index / SquadWidth;
			float Length = Index % SquadWidth;
		
			PlacedMembers++;
			FVector LocalOffset = FVector(Width, Length, 0.0f);
			LocalOffset *= BufferDistance;
			FVector FrontOffset = CenterOffset;
			FrontOffset.X = 0.f;
			LocalOffset -= FrontOffset;
			LocalOffset = LocalOffset.RotateAngleAxis(180.f, FVector::UpVector);
			OutNewOffsets.Add(LocalOffset);
			Index++;
		}
	}	
	
	if (FormationType == EFormationType::Triangle)
	{
		OutNewOffsets.Empty();
		const auto& [SquadWidth, BufferDistance] = SquadFragment.FormationSettings;
		const float Spacing = BufferDistance;

		// Determine how many rows we need so that the triangular number >= Count.
		// Row 0 (front/tip) has 1 unit, row 1 has 2, row 2 has 3, etc.
		// Total slots for NumRows rows = NumRows * (NumRows + 1) / 2
		int32 NumRows = 1;
		while (NumRows * (NumRows + 1) / 2 < Count)
		{
			NumRows++;
		}

		// Place units row by row, front (tip) to back (widest).
		// Row i has (i + 1) slots.
		int32 PlacedMembers = 0;
		for (int32 Row = 0; Row < NumRows && PlacedMembers < Count; ++Row)
		{
			const int32 UnitsInRow = Row + 1;
			// Center the row laterally
			const float RowLateralStart = -static_cast<float>(UnitsInRow - 1) * 0.5f * Spacing;

			for (int32 Col = 0; Col < UnitsInRow && PlacedMembers < Count; ++Col)
			{
				// X = depth (row 0 is at 0, subsequent rows move backward)
				const float DepthOffset = static_cast<float>(Row) * Spacing;
				const float LateralOffset = RowLateralStart + static_cast<float>(Col) * Spacing;

				FVector LocalOffset = FVector(DepthOffset, LateralOffset, 0.0f);
				LocalOffset = LocalOffset.RotateAngleAxis(180.f, FVector::UpVector);
				OutNewOffsets.Add(LocalOffset);
				PlacedMembers++;
			}
		}		
	}
	// Compute the centroid of all raw offsets and re-center them
	if (OutNewOffsets.Num() > 0)
	{
		FVector Centroid = FVector::ZeroVector;
		for (const FVector& Offset : OutNewOffsets)
		{
				Centroid += Offset;
		}
		Centroid /= static_cast<float>(OutNewOffsets.Num());

		SquadFragment.SquadCentroid = Centroid;

		for (FVector& Offset : OutNewOffsets)
		{
			Offset -= Centroid;
		}
	}
}
