//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "Camera/LyraCameraMode_CruorRTS_Player.h"

#include "Camera/LyraCameraComponent.h"
#include "PlayerPawn/CruorRTS_PlayerPawnHeroComponent.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"

ULyraCameraMode_CruorRTS_Player::ULyraCameraMode_CruorRTS_Player()
{
	ZoomPostProcessing.bOverride_DepthOfFieldFstop = true;
	ZoomPostProcessing.bOverride_DepthOfFieldSensorWidth = true;
	ZoomPostProcessing.bOverride_DepthOfFieldFocalDistance = true;
	ZoomPostProcessing.bOverride_WhiteTemp = true;
	ZoomPostProcessing.bOverride_AutoExposureBias = true;
	ZoomPostProcessing.bOverride_BloomIntensity = true;
	ZoomPostProcessing.bOverride_ColorSaturation = true;
	ZoomPostProcessing.bOverride_ColorContrast = true;
	ZoomPostProcessing.bOverride_DepthOfFieldSqueezeFactor = true;
}

void ULyraCameraMode_CruorRTS_Player::UpdateView(float DeltaTime)
{
	const ACruorRTS_PlayerPawn* TargetPawn = Cast<ACruorRTS_PlayerPawn>(GetTargetActor());
	check(TargetPawn);
	const UCruorRTS_PlayerPawnHeroComponent* HeroComponent = TargetPawn->FindComponentByClass<UCruorRTS_PlayerPawnHeroComponent>();
	check(HeroComponent);

	const FVector PivotLocation = TargetPawn->GetPivotPoint()->GetComponentLocation();
	const FRotator PivotRotation = TargetPawn->
	GetPivotPoint()->GetComponentRotation()	+ 
		FRotator(FMath::Lerp(-ZoomedOutAngle, -ZoomedInAngle, HeroComponent->GetZoomPosition()), 0.f, 0.f);
	
	// View Settings
	View.Location = PivotLocation + PivotRotation.Vector() * 
		-FMath::Lerp(MaximumCameraDistance, MinimumCameraDistance, HeroComponent->GetZoomPosition()) + 
			PivotRotation.RotateVector(CameraPivotPointOffset);
	
	View.Rotation = PivotRotation;
	
	View.ControlRotation = View.Rotation;
	
	View.FieldOfView = FMath::Lerp(ZoomedOutFieldOfView, ZoomedInFieldOfView, HeroComponent->GetZoomPosition());
	 
	/* Post Processing Settings	*/
	ZoomPostProcessing.DepthOfFieldFstop = FMath::Lerp(DepthOfFieldFStopMaxZoom, 
			DepthOfFieldFStopMinZoom, HeroComponent->GetZoomPosition());		
	
	ZoomPostProcessing.DepthOfFieldSensorWidth = FMath::Lerp(DepthOfFieldSensorWidthMaxZoom, 
		DepthOfFieldSensorWidthMinZoom, HeroComponent->GetZoomPosition());
	
	// DoF Focal Distance is adjusted by another offset because the original CameraPivotPointOffset above
	ZoomPostProcessing.DepthOfFieldFocalDistance = (PivotRotation.Vector() * 
		-FMath::Lerp(DepthOfFieldFocalDistanceMaxZoom, DepthOfFieldFocalDistanceMinZoom, HeroComponent->GetZoomPosition())).Length() 
	+ FMath::Lerp(FocalDistanceOffsetMaxZoom,FocalDistanceOffsetMinZoom, HeroComponent->GetZoomPosition());
	
	ZoomPostProcessing.DepthOfFieldSqueezeFactor = FMath::Lerp(DepthOfFieldSqueezeFactorMaxZoom, 
		DepthOfFieldSqueezeFactorMinZoom, HeroComponent->GetZoomPosition());
	
	ZoomPostProcessing.WhiteTemp = WhiteTemperature;
	ZoomPostProcessing.AutoExposureBias = AutoExposureBias;
	ZoomPostProcessing.BloomIntensity = BloomIntensity;
	ZoomPostProcessing.ColorSaturation = ColourSaturation;
	ZoomPostProcessing.ColorContrast = ColourContrast;
	
	// // Uncomment for debugging purposes
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT("FStop: %f, SensorWidth: %f, FDist: %f, FSqueeze: %f"), ZoomPostProcessing.DepthOfFieldFstop, ZoomPostProcessing.DepthOfFieldSensorWidth, ZoomPostProcessing.DepthOfFieldFocalDistance, ZoomPostProcessing.DepthOfFieldSqueezeFactor));
		
	GetLyraCameraComponent()->PostProcessSettings = ZoomPostProcessing;
}
