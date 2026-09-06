// Copyright Cruor enf., All Rights Reserved.


#include "Interaction/Mass/CruorRTS_MassSelectorSubsystem.h"

#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Kismet/GameplayStatics.h"

void UCruorRTS_MassSelectorSubsystem::UpdateLocalPlayerSelectorPosition(UWorld* World)
{
	if (!World) return;

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController || !PlayerController->IsLocalController()) return;
	
	if (const ACruorRTS_PlayerPawn* CommanderPawn = Cast<ACruorRTS_PlayerPawn>(PlayerController->GetPawn()))
	{
		CachedLocalPlayerSelectorPosition = CommanderPawn->GetSelector()->GetComponentLocation();
	}
}
