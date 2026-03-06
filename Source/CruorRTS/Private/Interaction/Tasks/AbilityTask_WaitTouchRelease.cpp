//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "Interaction/Tasks/AbilityTask_WaitTouchRelease.h"

#include "CruorRTS/CruorRTSLogChannels.h"

void UAbilityTask_WaitTouchRelease::OnTouchReleased()
{
	OnTouchRelease.Broadcast();
	EndTask();
}

void UAbilityTask_WaitTouchRelease::Activate()
{
	SetWaitingOnAvatar();
	
	const UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::CheckIfTouch, TouchPollingRate, true);
}

UAbilityTask_WaitTouchRelease* UAbilityTask_WaitTouchRelease::WaitTouchRelease(UGameplayAbility* OwningAbility,
	const float TouchPollingRate)
{
	UAbilityTask_WaitTouchRelease* MyObj = NewAbilityTask<UAbilityTask_WaitTouchRelease>(OwningAbility);
	MyObj->TouchPollingRate = TouchPollingRate;
	return MyObj;
}

void UAbilityTask_WaitTouchRelease::OnDestroy(bool AbilityEnded)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	
	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_WaitTouchRelease::CheckIfTouch()
{
	const APawn* OwningPawn = Cast<APawn>(GetAvatarActor());
	if (!OwningPawn)
	{
		UE_LOG(LogCruorRTS, Warning, TEXT("AbilityTask_WaitTouchRelease: OwningPawn not found."))
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(OwningPawn->GetController());
	if (!PlayerController)
	{
		UE_LOG(LogCruorRTS, Warning, TEXT("AbilityTask_WaitTouchRelease: PlayerController not found."))		
		return;
	}

	float OutX, OutY;
	bool bIsTouch1Pressed, bIsTouch2Pressed;
	PlayerController->GetInputTouchState(ETouchIndex::Type::Touch1, OutX, OutY, bIsTouch1Pressed);
	PlayerController->GetInputTouchState(ETouchIndex::Type::Touch2, OutX, OutY, bIsTouch2Pressed);
	
	if (bIsTouch1Pressed && !bIsTouch2Pressed)
	{
		return;
	}
	
	OnTouchReleased();
}
