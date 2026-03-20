// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "InteractionStatics.generated.h"

template <typename InterfaceType> class TScriptInterface;

class AActor;
class IInteractableTarget;
class UObject;
struct FFrame;
struct FHitResult;
struct FOverlapResult;

/**  */
UCLASS()
/* 
 * Start @CruorRTS_CodeModification
 * This is to allow access in CruorRTSPlayerGameplayAbility_Interact that is derived from LyraGameplayAbility_Interact which uses this class.
 * This is also directly used by CruorRTS_PlayerPawnMovementComponent and AbilityTask_GrantAbilitiesAndInteractionsOnHover.
*/
class LYRAGAME_API UInteractionStatics : public UBlueprintFunctionLibrary
/* 
 * End @CruorRTS_CodeModification 
*/
{
	GENERATED_BODY()

public:
	UInteractionStatics();

public:
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	UFUNCTION(BlueprintCallable)
	static void GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	static void AppendInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);
	static void AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);
};
