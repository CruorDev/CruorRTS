//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitTouchRelease.generated.h"

#define UE_API CRUORRTS_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTouchReleaseDelegate, float, TimeHeld);

/**
 * An Ability Task to determine if Touch1 has been released because AbilityTask_WaitInputRelease is bugged (UE_5.7)
 */
UCLASS(MinimalAPI)
class UAbilityTask_WaitTouchRelease : public UAbilityTask
{
	GENERATED_BODY()
	
	UE_API explicit UAbilityTask_WaitTouchRelease(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable)
	FTouchReleaseDelegate OnTouchRelease;
	
	UFUNCTION()
	void OnTouchReleased();
	
	virtual void Activate() override;

	/** Wait until touch is released.  This task automatically loops. */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitTouchRelease* WaitTouchRelease(UGameplayAbility* OwningAbility, const float TouchPollingRate = 0.100);

private:

	virtual void OnDestroy(bool AbilityEnded) override;
	
	void CheckIfTouch();

	float TouchPollingRate = 0.100;
	
	float StartTime;

	FTimerHandle TimerHandle;
};

#undef UE_API
