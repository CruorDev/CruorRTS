//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#pragma once

#include "Character/LyraHeroComponent.h"

#include "CruorRTS_PlayerPawnHeroComponent.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

UCLASS(MinimalAPI, Blueprintable, meta=(BlueprintSpawnableComponent))
class UCruorRTS_PlayerPawnHeroComponent : public ULyraHeroComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API explicit UCruorRTS_PlayerPawnHeroComponent(const FObjectInitializer& ObjectInitializer);	
	
	// Used for camera Zoom
	UE_API float GetZoomPosition() const { return ZoomPosition; }

protected:

	UE_API virtual void OnRegister() override;
	// Add custom Native InputTag Actions
	UE_API virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent) override;

	// Handle custom Native InputActions
	UE_API void Input_Move(const FInputActionValue& InputActionValue);
	UE_API void Input_Spin(const FInputActionValue& InputActionValue);
	UE_API void Input_DragMove(const FInputActionValue& InputActionValue);
	UE_API void Input_DragMoveStarted(const FInputActionValue& InputActionValue);
	UE_API void Input_Zoom(const FInputActionValue& InputActionValue);
	UE_API void Input_TouchZoomAndSpin(const FInputActionValue& InputActionValue);
	UE_API void Input_TouchZoomAndSpinStarted(const FInputActionValue& InputActionValue);
	UE_API void Input_TouchZoomAndSpinCompleted(const FInputActionValue& InputActionValue);
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Zoom Settings")
	FRuntimeFloatCurve ZoomCurve;
	
private:
	
	void UpdateZoomPosition(const float ZoomDirection);	
	float ZoomPosition;
	float ZoomValue = 0.5f;
	float ZoomedInMaxSpeed = 1000.f;
	float ZoomedOutMaxSpeed = 10000.f;
	
private:
	
	FVector DragMoveStartLocation;
	float OldTouch2LocationX;
	float OldTouch2LocationY;
};

#undef UE_API

