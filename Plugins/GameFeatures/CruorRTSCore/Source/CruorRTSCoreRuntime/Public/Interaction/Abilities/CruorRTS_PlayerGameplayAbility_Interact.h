//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Abilities/LyraGameplayAbility_Interact.h"
#include "CruorRTS_PlayerGameplayAbility_Interact.generated.h"

/**
 * 
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UCruorRTS_PlayerGameplayAbility_Interact : public ULyraGameplayAbility_Interact
{
	GENERATED_BODY()
	
	explicit UCruorRTS_PlayerGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
		
};
