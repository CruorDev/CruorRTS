//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "Interaction/Abilities/CruorRTS_PlayerGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "Interaction/InteractionStatics.h"
#include "NativeGameplayTags.h"
#include "Interaction/IInteractableTarget.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AI_Ability_FromMass, "AI.Ability.FromMass");

UCruorRTS_PlayerGameplayAbility_Interact::UCruorRTS_PlayerGameplayAbility_Interact(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCruorRTS_PlayerGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UCruorRTS_PlayerGameplayAbility_Interact::MassTriggerInteraction(const FInteractionOption& Option)
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AActor* Instigator = GetAvatarActorFromActorInfo();	
		FInteractionOption InteractionOption = Option;		
		FGameplayEventData MassPayload;
		MassPayload.EventTag = TAG_Ability_Interaction_Activate;
		MassPayload.Instigator = Instigator;
		MassPayload.Target = Instigator;
		MassPayload.TargetTags.AddTag(TAG_AI_Ability_FromMass);		
		if (InteractionOption.InteractionAbilityToGrant)
		{
			if (!InteractionOption.TargetInteractionAbilityHandle.IsValid())
			{
				if (const FObjectKey ObjectKey(InteractionOption.InteractionAbilityToGrant); !InteractionAbilityCache.Find(ObjectKey))
				{
					const FGameplayAbilitySpec Spec(InteractionOption.InteractionAbilityToGrant, 1, INDEX_NONE, this);
					const FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
					InteractionAbilityCache.Add(ObjectKey, Handle);
				}
			}
			if (const FGameplayAbilitySpec* InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InteractionOption.InteractionAbilityToGrant))
			{
				InteractionOption.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
			}		
		}
			
		FGameplayAbilityActorInfo MassActorInfo;
		MassActorInfo.InitFromActor(Instigator, Instigator, AbilitySystemComponent);
			
		const bool bSuccess = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&MassActorInfo,
			TAG_Ability_Interaction_Activate,
			&MassPayload,
			*AbilitySystemComponent
			);
	}
}

