//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Mass/EntityElementTypes.h"
#include "MassMovementTypes.h"
#include "CruorRTS_SquadFragments.generated.h"

UENUM(BlueprintType)
enum class EFormationType : uint8
{
	Rectangle,
	Triangle
};

/**
 * Defines how a Unique Squad is identified.
 */
USTRUCT(BlueprintType)
struct FFormationSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	uint8 SquadWidth = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	float BufferDistance = 300.f;
};

/**
 * Defines a squad's current status.
 */
USTRUCT(BlueprintType)
struct FCruorRTS_SquadFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FCruorRTS_SquadFragment() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	FFormationSettings FormationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	EFormationType FormationType = EFormationType::Rectangle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	FVector SquadDestination = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	FQuat SquadRotation = FQuat::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 ActiveMemberCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	FVector SquadCentroid = FVector::ZeroVector;
};

/**
 * Defines an individual squad member.
 */
USTRUCT(BlueprintType)
struct FCruorRTS_SquadMemberFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FCruorRTS_SquadMemberFragment() = default;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	FMassEntityHandle SquadEntity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	FVector Offset = FVector::ZeroVector;
};

USTRUCT()
struct FCruorRTS_SquadMemberMovementSettingsFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "SquadMember")
	FMassMovementStyleRef WalkMovement;

	UPROPERTY(EditAnywhere, Category = "SquadMember")
	FMassMovementStyleRef RunMovement;	
};
