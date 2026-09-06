// Copyright Cruor enf., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassSubsystemBase.h"
#include "SquadMovement/CruorRTS_SquadFragments.h"
#include "Interaction/InteractionOption.h"
#include "UI/Menu/CruorRTS_WheelMenu_GlyphIconData.h"
#include "CruorRTS_MassSelectorSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTS_MassSelectorSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()
	
public:
	
	void UpdateLocalPlayerSelectorPosition(UWorld* World);
	
	FVector GetCachedLocalPlayerPosition() const { return CachedLocalPlayerSelectorPosition; }
	
	UFUNCTION(BlueprintCallable)
	TArray<FInteractionOption> GetCurrentOptions() const { return CurrentOptions; }
	
	void ResetCurrentOptions() { CurrentOptions.Empty(); }
	
	void AddCurrentOption(const FInteractionOption& NewOption) { CurrentOptions.Add(NewOption); }
	
	void SetCachedSquadHandle(const FMassEntityHandle& SquadHandle) { CachedSquadHandle = SquadHandle; }
	
	UFUNCTION(BlueprintCallable)
	FMassEntityHandle GetCachedSquadHandle() const { return CachedSquadHandle; }
	
	void SetCachedWheelMenuGlyphIconData(const TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> InnerWheel, const TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> OuterWheel) { CachedInnerWheelMenuGlyphIconData = InnerWheel; CachedOuterWheelMenuGlyphIconData = OuterWheel; }
	
	UFUNCTION(BlueprintCallable)
	UCruorRTS_WheelMenu_GlyphIconData* GetInnerWheelMenuGlyphIconData() const { return CachedInnerWheelMenuGlyphIconData; }
	
	UFUNCTION(BlueprintCallable)	
	UCruorRTS_WheelMenu_GlyphIconData* GetOuterWheelMenuGlyphIconData() const { return CachedOuterWheelMenuGlyphIconData; }
	
private:
	
	FVector CachedLocalPlayerSelectorPosition;
	
	TArray<FInteractionOption> CurrentOptions;
	
	UPROPERTY(Transient)
	FMassEntityHandle CachedSquadHandle;
	
	UPROPERTY(Transient)
	TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> CachedInnerWheelMenuGlyphIconData;
	
	UPROPERTY(Transient)
	TObjectPtr<UCruorRTS_WheelMenu_GlyphIconData> CachedOuterWheelMenuGlyphIconData;
};
