//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "Interaction/Tasks/AbilityTask_GrantAbilitiesAndInteractionsOnHover.h"

#include "AbilitySystemComponent.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionStatics.h"
#include "Physics/LyraCollisionChannels.h"
#include "Engine/OverlapResult.h"

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::Activate()
{
	SetWaitingOnAvatar();

	const UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(
		QueryTimerHandle, 
		this,
		&ThisClass::QueryInteractables, 
		InteractionScanRate, 
		true);
}

UAbilityTask_GrantAbilitiesAndInteractionsOnHover* UAbilityTask_GrantAbilitiesAndInteractionsOnHover::
GrantAbilitiesAndInteractions(UGameplayAbility* OwningAbility, FInteractionQuery InteractionQuery,
	float InteractionScanRange, float InteractionScanRate, bool bShowDebug)
{
	UAbilityTask_GrantAbilitiesAndInteractionsOnHover* MyObj = 
		NewAbilityTask<UAbilityTask_GrantAbilitiesAndInteractionsOnHover>(OwningAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->InteractionQuery = InteractionQuery;
	MyObj->bShowDebug = bShowDebug;
	
	return MyObj;
}

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::UpdateInteractableOptions(
	const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (const TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
	{
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, TempOptions);
		InteractiveTarget->GatherInteractionOptions(InteractQuery, InteractionBuilder);

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;

			// if there is a handle an a target ability system, we're triggering the ability on the target.
			if (Option.TargetAbilitySystem && Option.TargetInteractionAbilityHandle.IsValid())
			{
				// Find the spec
				InteractionAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
			}
			// If there's an interaction ability then we're activating it on ourselves.
			else if (Option.InteractionAbilityToGrant)
			{
				// Find the spec
				InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.InteractionAbilityToGrant);

				if (InteractionAbilitySpec)
				{
					// update the option
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
				}
			}

			if (InteractionAbilitySpec)
			{
				// Filter any options that we can't activate right now for whatever reason.
				if (InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();

		for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}
	else
	{
		bOptionsChanged = true;
	}

	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		InteractableObjectsChanged.Broadcast(CurrentOptions);
	}
}

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::OnDestroy(bool AbilityEnded)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}
	
	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::QueryInteractables()
{
	FollowCursor();

	const UWorld* World = GetWorld();
	AActor* ActorOwner = GetAvatarActor();
	
	if (!World || !ActorOwner)
	{
		return;
	}

	const FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantAbilitiesAndInteractionsOnHover), false);
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(OUT OverlapResults, 
		SelectionProjection, 
		FQuat::Identity, 
		Lyra_TraceChannel_Interaction, 
		FCollisionShape::MakeSphere(InteractionScanRange), Params);

	if (OverlapResults.Num() > 0)
	{
		TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
		UInteractionStatics::AppendInteractableTargetsFromOverlapResults(OverlapResults, OUT InteractableTargets);
		GrantAbilities(InteractionQuery, InteractableTargets);
		UpdateInteractableOptions(InteractionQuery, InteractableTargets);		
	}
	
	
	
}

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::FollowCursor()
{
	if (const ACruorRTS_PlayerPawn* CommanderPawn = Cast<ACruorRTS_PlayerPawn>(GetAvatarActor()))
	{
		SelectionProjection = CommanderPawn->GetSelector()->GetComponentLocation();		
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("SelectionProjection: %s"), *SelectionProjection.ToString()));
	
#if ENABLE_DRAW_DEBUG
		if (const UWorld* World = GetWorld(); World && bShowDebug)
		{
			DrawDebugSphere(World, SelectionProjection, 
						InteractionScanRange, 4, FColor::Red, 
						false, InteractionScanRate);
		}
#endif
	}
}

void UAbilityTask_GrantAbilitiesAndInteractionsOnHover::GrantAbilities(const FInteractionQuery& InteractQuery, TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> Options;
	for (TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
	{
		FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, Options);
		InteractiveTarget->GatherInteractionOptions(InteractQuery, InteractionBuilder);
	}

	// Check if any of the options need to grant the ability to the user before they can be used.
	for (FInteractionOption& Option : Options)
	{
		if (Option.InteractionAbilityToGrant)
		{
			// Grant the ability to the GAS, otherwise it won't be able to do whatever the interaction is.
			FObjectKey ObjectKey(Option.InteractionAbilityToGrant);
			if (!InteractionAbilityCache.Find(ObjectKey))
			{
				FGameplayAbilitySpec Spec(Option.InteractionAbilityToGrant, 1, INDEX_NONE, this);
				FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
				InteractionAbilityCache.Add(ObjectKey, Handle);
			}
		}
	}
}
