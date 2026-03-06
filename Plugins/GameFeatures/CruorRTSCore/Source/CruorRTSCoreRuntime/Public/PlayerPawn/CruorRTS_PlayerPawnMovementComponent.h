//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "CruorRTS_PlayerPawnMovementComponent.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

enum class ECommonInputType : uint8;

UCLASS(MinimalAPI, Config = Game)
class UCruorRTS_PlayerPawnMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	explicit UCruorRTS_PlayerPawnMovementComponent(const FObjectInitializer& ObjectInitializer);	

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;	
	
private:
	
	void MoveTracking();
	void UpdatePivotPoint() const;
	void UpdateCursorProjection() const;
	

private:
	
	/**Current location of Mouse/Touch or Center of Screen if using Gamepad on the Custom Landscape Channel, updated by MoveTracking()*/
	FVector ScreenLocationIntersection;
	
	FVector DragMoveLocation;
};

#undef UE_API
