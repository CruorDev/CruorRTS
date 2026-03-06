//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"
#include "Interaction/IInteractableTarget.h"
#include "CruorRTS_CharacterWithAbilitiesBase.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class ULyraPawnData;

UCLASS(MinimalAPI, Blueprintable)
class ACruorRTS_CharacterWithAbilitiesBase : public ALyraCharacter, public IInteractableTarget
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	UE_API explicit ACruorRTS_CharacterWithAbilitiesBase(const FObjectInitializer& ObjectInitializer);
	
	UE_API virtual void PostInitializeComponents() override;

	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UE_API virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;
	
	static const FName NAME_LyraAbilityReady;
	
	void SetPawnData(const ULyraPawnData* InPawnData);

protected:
	
	virtual void OnAbilitySystemInitialized() override;
	
	UPROPERTY(EditAnywhere)
	FInteractionOption Option;
	
private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Lyra|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
	
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraHealthSet> HealthSet;
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraCombatSet> CombatSet;
};

#undef UE_API
