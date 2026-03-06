//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "CommonInputSubsystem.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Camera/LyraCameraComponent.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "PlayerPawn/CruorRTS_PlayerPawnMovementComponent.h"
#include "System/LyraSignificanceManager.h"


ACruorRTS_PlayerPawn::ACruorRTS_PlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(RootSceneComponent);
	
	PivotPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PivotPoint"));
	PivotPoint->SetupAttachment(GetRootComponent());

	Selector = CreateDefaultSubobject<USceneComponent>(TEXT("Selector"));
	Selector->SetupAttachment(GetRootComponent());
	
	MovementComponent = CreateDefaultSubobject<UCruorRTS_PlayerPawnMovementComponent>(TEXT("CruorRTSPlayerPawnMovementComponent"));
	MovementComponent->SetUpdatedComponent(GetRootComponent());
	
	PawnExtensionComponentComponent = CreateDefaultSubobject<ULyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtensionComponentComponent->OnAbilitySystemInitialized_RegisterAndCall(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtensionComponentComponent->OnAbilitySystemUninitialized_Register(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	
	CameraComponent = CreateDefaultSubobject<ULyraCameraComponent>(TEXT("CameraComponent"));	
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
	
	CursorProjection = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorProjection"));
	CursorProjection->SetupAttachment(GetRootComponent());
}

ALyraPlayerController* ACruorRTS_PlayerPawn::GetLyraPlayerController() const
{
	return Cast<ALyraPlayerController>(GetController());
}

ALyraPlayerState* ACruorRTS_PlayerPawn::GetLyraPlayerState() const
{
	return CastChecked<ALyraPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

ULyraAbilitySystemComponent* ACruorRTS_PlayerPawn::GetLyraAbilitySystemComponent() const
{
	return Cast<ULyraAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ACruorRTS_PlayerPawn::GetAbilitySystemComponent() const
{
	if (PawnExtensionComponentComponent == nullptr)
	{
		return nullptr;
	}
	
	return PawnExtensionComponentComponent->GetLyraAbilitySystemComponent();
}

void ACruorRTS_PlayerPawn::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const ULyraAbilitySystemComponent* LyraAbilitySystemComponent = GetLyraAbilitySystemComponent())
	{
		LyraAbilitySystemComponent->GetOwnedGameplayTags(TagContainer);;
	}
}

bool ACruorRTS_PlayerPawn::HasMatchingGameplayTag(const FGameplayTag TagToCheck) const
{
	if (const ULyraAbilitySystemComponent* LyraAbilitySystemComponent = GetLyraAbilitySystemComponent())
	{
		return LyraAbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}
	
	return false;
}

bool ACruorRTS_PlayerPawn::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const ULyraAbilitySystemComponent* LyraAbilitySystemComponent = GetLyraAbilitySystemComponent())
	{
		return LyraAbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}
	
	return false;
}

bool ACruorRTS_PlayerPawn::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const ULyraAbilitySystemComponent* LyraAbilitySystemComponent = GetLyraAbilitySystemComponent())
	{
		return LyraAbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}
	
	return false;
}

void ACruorRTS_PlayerPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ACruorRTS_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	
	if (!IsNetMode(NM_DedicatedServer))
	{
		if (ULyraSignificanceManager* SignificanceManager = USignificanceManager::Get<ULyraSignificanceManager>(World))
		{
			//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		}
	}
}

void ACruorRTS_PlayerPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const UWorld* World = GetWorld();

	if (!IsNetMode(NM_DedicatedServer))
	{
		if (ULyraSignificanceManager* SignificanceManager = USignificanceManager::Get<ULyraSignificanceManager>(World))
		{
			SignificanceManager->UnregisterObject(this);
		}
	}
	
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UCommonInputSubsystem* CommonInputSubsystem = ULocalPlayer::GetSubsystem<UCommonInputSubsystem>(PlayerController->GetLocalPlayer()))
		{
			CommonInputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
		}
	}
}

void ACruorRTS_PlayerPawn::Reset()
{
	K2_OnReset();
	
	UnInitAndDestroy();
}

void ACruorRTS_PlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ACruorRTS_PlayerPawn::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	PawnExtensionComponentComponent->HandleControllerChanged();

	// Grab the current team ID and listen for future changes
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
	
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UCommonInputSubsystem* CommonInputSubsystem = ULocalPlayer::GetSubsystem<UCommonInputSubsystem>(PlayerController->GetLocalPlayer()))
		{
			CommonInputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::OnCommonInputTypeChanged);
			SetCurrentInputType(CommonInputSubsystem->GetCurrentInputType());
		}
	}
}

void ACruorRTS_PlayerPawn::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	PawnExtensionComponentComponent->HandleControllerChanged();

	// Determine what the new team ID should be afterward
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ACruorRTS_PlayerPawn::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our team ID based on the controller
	if (HasAuthority() && (GetController() != nullptr))
	{
		if (ILyraTeamAgentInterface* ControllerWithTeam = Cast<ILyraTeamAgentInterface>(GetController()))
		{
			MyTeamID = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
		}
	}
}

void ACruorRTS_PlayerPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	ILyraTeamAgentInterface::SetGenericTeamId(NewTeamID);
}

FGenericTeamId ACruorRTS_PlayerPawn::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnLyraTeamIndexChangedDelegate* ACruorRTS_PlayerPawn::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ACruorRTS_PlayerPawn::OnAbilitySystemInitialized()
{
	ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
	check(LyraASC);

	InitializeGameplayTags();
}

void ACruorRTS_PlayerPawn::OnAbilitySystemUninitialized()
{
}

void ACruorRTS_PlayerPawn::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	PawnExtensionComponentComponent->HandleControllerChanged();
}

void ACruorRTS_PlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	PawnExtensionComponentComponent->HandlePlayerStateReplicated();
}

void ACruorRTS_PlayerPawn::InitializeGameplayTags()
{
	
}

void ACruorRTS_PlayerPawn::UnInitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (const ULyraAbilitySystemComponent* LyraAbilitySystemComponent = GetLyraAbilitySystemComponent())
	{
		if (LyraAbilitySystemComponent->GetAvatarActor() == this)
		{
			PawnExtensionComponentComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void ACruorRTS_PlayerPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, const int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ACruorRTS_PlayerPawn::OnRep_MyTeamID(const FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ACruorRTS_PlayerPawn::OnCommonInputTypeChanged(const ECommonInputType NewType)
{
	SetCurrentInputType(NewType);
}

void ACruorRTS_PlayerPawn::SetCurrentInputType(const ECommonInputType InType)
{
	CurrentInputType = InType;
	
	CurrentInputType == ECommonInputType::Touch ? CursorProjection->SetVisibility(false) : CursorProjection->SetVisibility(true);
}

// Called to bind functionality to input
void ACruorRTS_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PawnExtensionComponentComponent->SetupPlayerInputComponent();
}

