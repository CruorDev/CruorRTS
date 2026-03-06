//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/InteractionOption.h"
#include "Interaction/InteractionQuery.h"
#include "AbilityTask_GrantAbilitiesAndInteractionsOnHover.generated.h"

class IInteractableTarget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&,
                                            InteractableOptions);

/**
 * Follows the Commander's Cursor; Grants Gameplay Abilities and Creates Interactable Options for EbonradeCommanderGameplayAbility_Interact to use.
 */
UCLASS()
class CRUORRTSCORERUNTIME_API UAbilityTask_GrantAbilitiesAndInteractionsOnHover : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	
	virtual void Activate() override;
	
	UFUNCTION(BlueprintCallable, 
		Category="Ability|Tasks", 
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantAbilitiesAndInteractionsOnHover* GrantAbilitiesAndInteractions(
		UGameplayAbility* OwningAbility, 
		FInteractionQuery InteractionQuery, 
		float InteractionScanRange, 
		float InteractionScanRate, 
		bool bShowDebug = false);
	
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;
	
protected:
	
	void UpdateInteractableOptions(
		const FInteractionQuery& InteractQuery, 
		const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets);
	
	TArray<FInteractionOption> CurrentOptions;
	
	FCollisionProfileName TraceProfile;
	
private:

	virtual void OnDestroy(bool AbilityEnded) override;

	void QueryInteractables();
	void FollowCursor();
	void GrantAbilities(
		const FInteractionQuery& InteractQuery, 
		TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets);
	
	UPROPERTY()
	FInteractionQuery InteractionQuery;
	
	FVector SelectionProjection;

	float InteractionScanRange = 50.f;
	float InteractionScanRate = 0.100f;
	bool bShowDebug = false;

	FTimerHandle QueryTimerHandle;

	TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};
