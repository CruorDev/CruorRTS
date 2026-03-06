//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerSpawningManagerComponent.h"
#include "CruorRTS_PlayerPawnSpawningManagementComponent.generated.h"


UCLASS(ClassGroup=(CruorRTS_PlayerPawn), meta=(BlueprintSpawnableComponent))
class CRUORRTSCORERUNTIME_API
	UCruorRTS_PlayerPawnSpawningManagementComponent : public ULyraPlayerSpawningManagerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	explicit UCruorRTS_PlayerPawnSpawningManagementComponent(const FObjectInitializer& ObjectInitializer);
	
	// ULyraPlayerSpawningManagerComponent
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts) override;
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) override;
	// ~ULyraPlayerSpawningManagerComponent
};
