//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "PlayerPawn/CruorRTS_UnitLeaderSpawnerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/CruorRTS_UnitLeaderSpawnerPawnData.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Teams/LyraTeamSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerState.h"

#define COLLISION_LANDSCAPE ECC_GameTraceChannel6

/* 
 * Start @CruorRTS_CodeModification
 * The CruorRTS_PlayerPawn that this attached to is locked to a plane so spawning Z-height needs to be adjust as to not spawn inside terrain.
*/
namespace CruorRTS_UnitLeaderSpawner
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("CruorRTS_UnitLeaderSpawner.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace up when updating the Spawned Pawn's z-height."), ECVF_Cheat);
}
/* 
 *  End @CruorRTS_CodeModification
*/

UCruorRTS_UnitLeaderSpawnerComponent::UCruorRTS_UnitLeaderSpawnerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCruorRTS_UnitLeaderSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();	
	
	// Listen for the experience load to complete
	const AGameStateBase* GameState = GetWorld()->GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	/* 
	 * Start @CruorRTS_CodeModification
	 * The CruorRTS_PlayerPawn determines what Team to spawn the Unit Leader on. 
	*/
	ACruorRTS_PlayerPawn* OwningCommander = GetOwner<ACruorRTS_PlayerPawn>();
	check(OwningCommander);
	OwningCommander->GetOnTeamIndexChangedDelegate()->AddDynamic(this, &ThisClass::OnTeamAssigned);
	/* 
	 * End @CruorRTS_CodeModification
	*/
}



void UCruorRTS_UnitLeaderSpawnerComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (GetOwner()->HasAuthority())
	{
		ServerSpawnUnitLeaders();
	}
#endif
}

/* 
 * Start @CruorRTS_CodeModification
 * The CruorRTS_PlayerPawn determines what Team to spawn the Unit Leader on. 
*/
void UCruorRTS_UnitLeaderSpawnerComponent::OnTeamAssigned(UObject* TeamAgent, int32 OldTeamID, int32 NewTeamID)
{
	if (OldTeamID == NewTeamID) return;
	TeamID = NewTeamID;
	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{
		for (const auto UnitLeader : UnitLeaderControllers)
		{		
			TeamSubsystem->ChangeTeamForActor(UnitLeader, TeamID);
		}		
	}	
}
/* 
 * End @CruorRTS_CodeModification
*/

#if WITH_SERVER_CODE
void UCruorRTS_UnitLeaderSpawnerComponent::ServerSpawnUnitLeaders_Implementation()
{
	for (const auto UnitLeader : UnitLeaders)
	{
		const UCruorRTS_UnitLeaderSpawnerPawnData* UnitLeaderData = UnitLeader.Get();
		if (!UnitLeaderData) UnitLeaderData = UnitLeader.LoadSynchronous();
		if (UnitLeaderData->PawnController == nullptr) continue;//@TODO debug this to check if working.
		SpawnOneUnitLeader(UnitLeader);
	}
}

void UCruorRTS_UnitLeaderSpawnerComponent::SpawnOneUnitLeader(const TSoftObjectPtr<UCruorRTS_UnitLeaderSpawnerPawnData> SpawningUnitLeaderData)
{
	const UCruorRTS_UnitLeaderSpawnerPawnData* NewUnitLeaderData = SpawningUnitLeaderData.Get();
	
	if (!NewUnitLeaderData)
	{
		NewUnitLeaderData = SpawningUnitLeaderData.LoadSynchronous();
	}
	/* 
	 * Start @CruorRTS_CodeModification
	 * The CruorRTS_PlayerPawn determines what Team to spawn the Unit Leader on. 
	*/
	FVector SpawnLocation;
	if (const UWorld* World = GetWorld())
	{
		
		FHitResult HitResultAboveRoot;
		World->LineTraceSingleByChannel(HitResultAboveRoot,
			GetOwner()->GetActorLocation() 
			+ GetOwner()->GetRootComponent()->GetUpVector() 
			* CruorRTS_UnitLeaderSpawner::GroundTraceDistance,
			GetOwner()->GetRootComponent()->GetComponentLocation() 
			+ GetOwner()->GetRootComponent()->GetUpVector() 
			* -CruorRTS_UnitLeaderSpawner::GroundTraceDistance,
			ECollisionChannel::COLLISION_LANDSCAPE);
		HitResultAboveRoot.bBlockingHit ? 
		SpawnLocation = HitResultAboveRoot.Location : 
		SpawnLocation = GetOwner()->GetActorLocation();
	}
	/* 
	 * End @CruorRTS_CodeModification 
	*/
	if (NewUnitLeaderData)
	{
		if (APawn* SpawnedUnitLeader = 
			SpawnUnitLeaderFromClass
			(
			GetWorld(), 
			NewUnitLeaderData, 
			SpawnLocation,
			GetOwner()->GetActorRotation(), 
			true, 
			GetOwner()
			)
			)
		{
			bool bWantsPlayerState = true;
			if (const AAIController* AIController = Cast<AAIController>(SpawnedUnitLeader->Controller))
			{
				bWantsPlayerState = AIController->bWantsPlayerState;
			}
			
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedUnitLeader))
			{
				AActor* AbilityOwner = bWantsPlayerState ? SpawnedUnitLeader->GetPlayerState() : Cast<AActor>(SpawnedUnitLeader);
				
				if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner))
				{
					PawnExtComp->InitializeAbilitySystem(Cast<ULyraAbilitySystemComponent>(AbilitySystemComponent), AbilityOwner);
				}
			}

			if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
			{
				TeamID = GetOwner<ACruorRTS_PlayerPawn>()->GetGenericTeamId();
				TeamSubsystem->ChangeTeamForActor(SpawnedUnitLeader->Controller, TeamID);
			}
			
			UnitLeaderControllers.Add(Cast<AAIController>(SpawnedUnitLeader->Controller));

			SpawnedUnitLeader->OnDestroyed.AddDynamic(this, &ThisClass::OnSpawnedPawnDestroyed);
		}
	}
}

APawn* UCruorRTS_UnitLeaderSpawnerComponent::SpawnUnitLeaderFromClass(const UObject* WorldContextObject,
	const UCruorRTS_UnitLeaderSpawnerPawnData* LoadedPawnData, const FVector& Location, const FRotator& Rotation, const bool bNoCollisionFail, AActor* PawnOwner)
{
	APawn* NewPawn = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull); World && LoadedPawnData->PawnData)
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.Owner = PawnOwner;
		ActorSpawnParams.ObjectFlags |= RF_Transient;	// We never want to save spawned AI pawns into a map
		ActorSpawnParams.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		// defer spawning the pawn to set up the AIController, else it spawns the default controller on spawn if set to spawn AI on spawn
		ActorSpawnParams.bDeferConstruction = LoadedPawnData->PawnController != nullptr;
		
		NewPawn = World->SpawnActor<APawn>(*LoadedPawnData->PawnData->PawnClass, Location, Rotation, ActorSpawnParams);
		if (*LoadedPawnData->PawnController)
		{
			NewPawn->AIControllerClass = LoadedPawnData->PawnController;
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
			{
				PawnExtComp->SetPawnData(LoadedPawnData->PawnData);
			}
			NewPawn->FinishSpawning(FTransform(Rotation, Location, FVector::OneVector));
		}
		
		if (NewPawn != nullptr)
		{
			if (NewPawn->Controller == nullptr)
			{
				// NOTE: SpawnDefaultController ALSO calls Possess() to possess the pawn (if a controller is successfully spawned).
				NewPawn->SpawnDefaultController();
			}
		}
	}

	return NewPawn;
}

void UCruorRTS_UnitLeaderSpawnerComponent::OnSpawnedPawnDestroyed(AActor* DestroyedPawn)
{
	if (!GetOwner()->HasAuthority()) return;
	
	UnitLeaderControllers.Remove(Cast<AAIController>(DestroyedPawn));
}

#else
void UCruorRTS_UnitLeaderSpawnerComponent::ServerSpawnUnitLeaders_Implementation()
{
	ensureMsgf(0, TEXT("SpawnUnitLeaders does not exist in LyraClient!"));
}

void UCruorRTS_UnitLeaderSpawnerComponent::SpawnOneUnitLeader(TSoftObjectPtr<UCruorRTS_UnitLeaderSpawnerPawnData>)()
{
	ensureMsgf(0, TEXT("SpawnUnitLeaders does not exist in LyraClient!"));
}
#endif


