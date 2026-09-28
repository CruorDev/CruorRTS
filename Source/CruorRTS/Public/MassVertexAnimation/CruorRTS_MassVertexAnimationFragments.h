//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once
#include "Mass/EntityFragments.h"

#include "CruorRTS_MassVertexAnimationFragments.generated.h"

class UAnimToTextureDataAsset;

USTRUCT()
struct FCruorRTS_MassVertexAnimationInfoFragment : public FMassFragment
{
	GENERATED_BODY()
	
	float GlobalStartTime = 0.0f;
	float PlayRate = 1.0f;
	int32 AnimationStateIndex = 0;
	bool bSwappedThisFrame = false;
};

USTRUCT()
struct FCruorRTS_MassVertexAnimationParametersFragment : public FMassConstSharedFragment
{	GENERATED_BODY()
	
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	TWeakObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;	
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	int16 SpeedThreshold = 50;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	int8 IdleAnimationIndex = 0;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	int8 WalkAnimationIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Animation")
	int8 RunAnimationIndex = 0;	
};
