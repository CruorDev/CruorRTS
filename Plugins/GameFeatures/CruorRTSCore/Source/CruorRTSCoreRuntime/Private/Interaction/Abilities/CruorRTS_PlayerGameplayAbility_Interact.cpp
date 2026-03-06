//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "Interaction/Abilities/CruorRTS_PlayerGameplayAbility_Interact.h"

UCruorRTS_PlayerGameplayAbility_Interact::UCruorRTS_PlayerGameplayAbility_Interact(
	const FObjectInitializer& ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCruorRTS_PlayerGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
