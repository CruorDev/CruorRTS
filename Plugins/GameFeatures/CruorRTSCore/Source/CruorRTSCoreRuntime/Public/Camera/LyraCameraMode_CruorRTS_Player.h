//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/LyraCameraMode.h"
#include "LyraCameraMode_CruorRTS_Player.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class ULyraCameraMode_CruorRTS_Player : public ULyraCameraMode
{
	GENERATED_BODY()
	
public:
	ULyraCameraMode_CruorRTS_Player();		
	
protected:
	
	virtual void UpdateView(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Length")
	float MaximumCameraDistance = 3500.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Length")
	float MinimumCameraDistance = 1500.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Angle")
	float ZoomedOutAngle = 30.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Angle")
	float ZoomedInAngle = 15.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Field of View")
	float ZoomedInFieldOfView = 50.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Zoom Properties|Field of View")
	float ZoomedOutFieldOfView = 50.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldFocalDistanceMaxZoom = MaximumCameraDistance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldFocalDistanceMinZoom = MinimumCameraDistance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldFStopMaxZoom = 10.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldFStopMinZoom = 16.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldSensorWidthMaxZoom = 250.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float DepthOfFieldSensorWidthMinZoom = 150.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Colour Temperature")
	float WhiteTemperature = 6500.f;	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Auto Exposure Bias")
	float AutoExposureBias = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Bloom Intensity")
	float BloomIntensity = 1.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Colour Saturation")
	FVector4 ColourSaturation = FVector4(1.f, 1.f, 1.f, 1.f);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Colour Contrast")
	FVector4 ColourContrast = FVector4(1.f, 1.f, 1.f, 1.f);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth Of Field Squeeze Factor")
	float DepthOfFieldSqueezeFactorMaxZoom = 5.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth Of Field Squeeze Factor")
	float DepthOfFieldSqueezeFactorMinZoom = 3.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Camera Pivot Point Offset")
	FVector CameraPivotPointOffset = FVector(-300.f, 0.f, 80.f);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float FocalDistanceOffsetMaxZoom = 800.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Properties|Depth of Field")
	float FocalDistanceOffsetMinZoom = 1000.f;
	
	FPostProcessSettings ZoomPostProcessing;
};

#undef UE_API
