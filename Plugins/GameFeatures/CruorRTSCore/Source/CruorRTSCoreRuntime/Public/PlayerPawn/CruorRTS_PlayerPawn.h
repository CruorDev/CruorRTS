//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularPawn.h"
#include "Teams/LyraTeamAgentInterface.h"
#include "CruorRTS_PlayerPawn.generated.h"

#define UE_API CRUORRTSCORERUNTIME_API

class UCruorRTS_PlayerPawnMovementComponent;
enum class ECommonInputType : uint8;
class ULyraAbilitySystemComponent;
class ALyraPlayerState;
class ALyraPlayerController;
class USphereComponent;
class ULyraCameraComponent;
class ULyraPawnExtensionComponent;

/*
 * @CruorRTS_CodeAttribution
 * 
 * This codes is an almost verbatim implementation of LyraCharacter from Copyright Epic Games, Inc., but based on AModularPawn
 */

UCLASS(MinimalAPI, Config = Game)
class ACruorRTS_PlayerPawn : public AModularPawn, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:
	
	UE_API explicit ACruorRTS_PlayerPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "CruorRTSPlayerPawn|PlayerPawn")
	UE_API ALyraPlayerController* GetLyraPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "CruorRTSPlayerPawn|PlayerPawn")
	UE_API ALyraPlayerState* GetLyraPlayerState() const;
	
	UFUNCTION(BlueprintCallable, Category = "CruorRTSPlayerPawn|PlayerPawn")
	UE_API ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UE_API ECommonInputType GetCurrentInputType() const { return CurrentInputType; }

	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	
	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	UE_API virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	//~ILyraTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface
	
	/* 
	 * Start @CruorRTS_CodeModification
	 * These are custom functions for CruorRTSPlayerPawn.
	*/
	UE_API USceneComponent* GetPivotPoint() const { return PivotPoint; }
	UE_API USceneComponent* GetSelector() const { return Selector; }
	UE_API UStaticMeshComponent* GetCursorProjection() const { return CursorProjection; }
	/* 
	 *  End @CruorRTS_CodeModification  
	*/

protected:
	
	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();
	
	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UE_API void InitializeGameplayTags();
	
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}	
	
	UE_API void UnInitAndDestroy();
	
private:
	
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);
	
	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);	
	
	/* 
	 * Start @CruorRTS_CodeModification
	 * These are custom functions and Properties for CruorRTSPlayerPawn.
	*/
	
	UFUNCTION()
	void OnCommonInputTypeChanged(const ECommonInputType NewType);
	
	void SetCurrentInputType(const ECommonInputType InType);
	
private:	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CruorRTSPlayerPawn|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtensionComponentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CruorRTSPlayerPawn|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCruorRTS_PlayerPawnMovementComponent> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CruorRTSPlayerPawn|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CruorRTSPlayerPawn|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RootSceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CruorRTSPlayerPawn|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PivotPoint;
	
	/**Collision that is projected either to center of screen or where mouse/touch is to allows DragMoving or Actor Selection*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,  Category = "CruorRTSPlayerPawn|Pawn",  meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Selector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Cursor, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CursorProjection;
	
	UPROPERTY()
	ECommonInputType CurrentInputType;
	
	/* 
	 *  End @CruorRTS_CodeModification  
	*/
	
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;
	
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;		
};

#undef UE_API
