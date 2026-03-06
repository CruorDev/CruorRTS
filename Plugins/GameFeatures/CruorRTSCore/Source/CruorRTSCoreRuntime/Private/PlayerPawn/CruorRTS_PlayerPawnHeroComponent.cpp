//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "PlayerPawn/CruorRTS_PlayerPawnHeroComponent.h"
#include "CommonInputSubsystem.h"
#include "CruorRTSCoreRuntime/CruorRTSCoreGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "CruorRTS/CruorRTSLogChannels.h"
#include "Input/LyraInputComponent.h"
#include "Player/LyraLocalPlayer.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"
#include "LyraGameplayTags.h"
#include "Camera/LyraCameraMode_CruorRTS_Player.h"
#include "PlayerPawn/CruorRTS_PlayerPawnMovementComponent.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Components/GameFrameworkComponentManager.h"
#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

#define COLLISION_LANDSCAPE ECC_GameTraceChannel6


UCruorRTS_PlayerPawnHeroComponent::UCruorRTS_PlayerPawnHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCruorRTS_PlayerPawnHeroComponent::OnRegister()
{
	Super::Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(
			LogCruorRTS, 
			Error, 
			TEXT("[UCruorRTSHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = 
				NSLOCTEXT("CruorRTSHeroComponent", 
				"NotOnPawnError", 
				"has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("CruorRTSHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

void UCruorRTS_PlayerPawnHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(PC->GetLocalPlayer());
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>())
		{
			if (const ULyraInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (const UInputMappingContext* InputMappingContext = Mapping.InputMapping.LoadSynchronous())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(InputMappingContext);
							}
							
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(InputMappingContext, Mapping.Priority, Options);
						}
					}
				}

				// The Lyra Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the ULyraInputComponent or modify this component accordingly.
				if (ULyraInputComponent* LyraInputComponent = Cast<ULyraInputComponent>(PlayerInputComponent); 
					ensureMsgf(LyraInputComponent, 
						TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					LyraInputComponent->AddInputMappings(InputConfig, Subsystem);
					
					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					LyraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
					
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_Spin, ETriggerEvent::Triggered, this, &ThisClass::Input_Spin, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_Zoom, ETriggerEvent::Triggered, this, &ThisClass::Input_Zoom, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_TouchZoomAndSpin, ETriggerEvent::Triggered, this, &ThisClass::Input_TouchZoomAndSpin, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_TouchZoomAndSpin, ETriggerEvent::Started, this, &ThisClass::Input_TouchZoomAndSpinStarted, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_TouchZoomAndSpin, ETriggerEvent::Completed, this, &ThisClass::Input_TouchZoomAndSpinCompleted, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_DragMove, ETriggerEvent::Triggered, this, &ThisClass::Input_DragMove, false);
					LyraInputComponent->BindNativeAction(InputConfig, CruorRTSCoreGameplayTags::InputTag_DragMove, ETriggerEvent::Started, this, &ThisClass::Input_DragMoveStarted, false);
				}
			}
		}
	}
	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}
 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
	UpdateZoomPosition(0.f);
}

void UCruorRTS_PlayerPawnHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (const AController* Controller = Pawn ? Pawn->GetController() : nullptr)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UCruorRTS_PlayerPawnHeroComponent::Input_Spin(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}	

	if (const float SpinAxisValue = InputActionValue.Get<float>(); SpinAxisValue != 0.0f)
	{
		Pawn->AddControllerYawInput(SpinAxisValue);
	}
}

void UCruorRTS_PlayerPawnHeroComponent::Input_DragMove(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (!PlayerController)
	{
		return;
	}	
	
	const UCommonInputSubsystem* CommonInputSubsystem = ULocalPlayer::GetSubsystem<UCommonInputSubsystem>(PlayerController->GetLocalPlayer());
	
	if (!CommonInputSubsystem)
	{
		return;
	}
	
	FVector StoredMove;
	const FVector CurrentLocation = Pawn->GetActorLocation();
	FVector TargetLocation;
	FHitResult HitResultSelector;
	int32 ScreenSizeX;
	int32 ScreenSizeY;
	PlayerController->GetViewportSize(ScreenSizeX, ScreenSizeY);
	FVector SelectionLocationIntersection; 
	float ScreenLocationX;
	float ScreenLocationY;
	
	switch (CommonInputSubsystem->GetCurrentInputType())
	{
		
		
		case ECommonInputType::MouseAndKeyboard:
		if (PlayerController->GetMousePosition(ScreenLocationX, ScreenLocationY))
		{
			PlayerController->GetHitResultUnderCursor(
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);
			SelectionLocationIntersection = HitResultSelector.Location;
			StoredMove = DragMoveStartLocation - SelectionLocationIntersection;
			TargetLocation = CurrentLocation + FVector(StoredMove.X, StoredMove.Y, 0.f);
			Pawn->SetActorLocation(FMath::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 15.f));
		}				
		break;
		case ECommonInputType::Touch:
		bool bTouch2CurrentlyPressed;
		bool bTouch1CurrentlyPressed;
		PlayerController->GetInputTouchState(ETouchIndex::Touch2, ScreenLocationX, ScreenLocationY, bTouch2CurrentlyPressed);
		PlayerController->GetInputTouchState(ETouchIndex::Touch1, ScreenLocationX, ScreenLocationY, bTouch1CurrentlyPressed);
		if (bTouch1CurrentlyPressed && !bTouch2CurrentlyPressed)
		{
			PlayerController->GetHitResultAtScreenPosition(
				FVector2D(ScreenLocationX, ScreenLocationY), 
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);
			SelectionLocationIntersection = HitResultSelector.Location;
			StoredMove = DragMoveStartLocation - SelectionLocationIntersection;
			TargetLocation = CurrentLocation + FVector(StoredMove.X, StoredMove.Y, 0.f);
			Pawn->SetActorLocation(FMath::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 15.f));
		}
		break;
		default:
		break;		
	}
}

void UCruorRTS_PlayerPawnHeroComponent::Input_DragMoveStarted(const FInputActionValue& InputActionValue)
{
	const APawn* Pawn = GetPawn<APawn>();
	
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (!PlayerController)
	{
		return;
	}	
	
	float ScreenLocationX;
	float ScreenLocationY;
	bool bTouch1CurrentlyPressed;
	PlayerController->GetInputTouchState(ETouchIndex::Touch1, ScreenLocationX, ScreenLocationY, bTouch1CurrentlyPressed);
	
	if (bTouch1CurrentlyPressed)
	{
		//Cast<UCruorRTSMovementComponent>(Pawn->GetMovementComponent())->UpdateMoveTracking();
		FHitResult HitResultSelector;
		PlayerController->GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::COLLISION_LANDSCAPE, false, HitResultSelector);
		DragMoveStartLocation = HitResultSelector.Location;
	}
	else
	{
		//const FVector SelectionLocationIntersection = Cast<UCruorRTSMovementComponent>(Pawn->GetMovementComponent())->GetScreenLocationIntersection();
		FHitResult HitResultSelector;
		int32 ScreenSizeX;
		int32 ScreenSizeY;
		PlayerController->GetViewportSize(ScreenSizeX, ScreenSizeY);
		const float CenterScreenX = static_cast<float>(ScreenSizeX / 2);
		const float CenterScreenY = static_cast<float>(ScreenSizeY / 2);
		FVector ScreenLocationIntersection;
		if (PlayerController->GetMousePosition(ScreenLocationX, ScreenLocationY))
		{
			PlayerController->GetHitResultUnderCursor(
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);
			ScreenLocationIntersection = HitResultSelector.Location;
		} 
		else
		{
			PlayerController->GetHitResultAtScreenPosition(
			FVector2D(CenterScreenX, CenterScreenY), 
			ECollisionChannel::COLLISION_LANDSCAPE, 
			false, 
			HitResultSelector);
		}
		
		ScreenLocationIntersection = HitResultSelector.Location;
		
		DragMoveStartLocation = ScreenLocationIntersection;
	}	
}

void UCruorRTS_PlayerPawnHeroComponent::Input_Zoom(const FInputActionValue& InputActionValue)
{
	if (const float ZoomDirection = InputActionValue.Get<float>() * 0.01f; ZoomDirection != 0.0f)
	{
		UpdateZoomPosition(ZoomDirection);
	}	
}

void UCruorRTS_PlayerPawnHeroComponent::Input_TouchZoomAndSpin(const FInputActionValue& InputActionValue)
{
	ACruorRTS_PlayerPawn* Pawn = GetPawn<ACruorRTS_PlayerPawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (!PlayerController)
	{
		return;
	}
	
	float LocationX, LocationY;
	bool bTouch2IsPressed;
	PlayerController->GetInputTouchState(ETouchIndex::Type::Touch2, LocationX, LocationY, bTouch2IsPressed);
	if (bTouch2IsPressed)
	{
		if (LocationY > 0.f)
		{
			UpdateZoomPosition((OldTouch2LocationY - LocationY) * 0.01f);
			OldTouch2LocationY = LocationY;
		}
		
		
		if (LocationX > 0.f)
		{
			Pawn->AddControllerYawInput((OldTouch2LocationX - LocationX) * 0.1f);
			OldTouch2LocationX = LocationX;
		}
		
	}
}

void UCruorRTS_PlayerPawnHeroComponent::Input_TouchZoomAndSpinStarted(const FInputActionValue& InputActionValue)
{
	ACruorRTS_PlayerPawn* Pawn = GetPawn<ACruorRTS_PlayerPawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (!PlayerController)
	{
		return;
	}	
	
	float LocationX, LocationY;
	bool bTouch2IsPressed;
	PlayerController->GetInputTouchState(ETouchIndex::Type::Touch2, LocationX, LocationY, bTouch2IsPressed);
	if (bTouch2IsPressed)
	{
		if (OldTouch2LocationX == 0.f && OldTouch2LocationY == 0.f)
		{
			OldTouch2LocationX = LocationX;
			OldTouch2LocationY = LocationY;
		}
	}
}

void UCruorRTS_PlayerPawnHeroComponent::Input_TouchZoomAndSpinCompleted(
	const FInputActionValue& InputActionValue)
{
	OldTouch2LocationX = OldTouch2LocationY = 0.f;
}

void UCruorRTS_PlayerPawnHeroComponent::UpdateZoomPosition(const float ZoomDirection)
{
	ZoomValue = FMath::Clamp(ZoomDirection + ZoomValue, 0.f, 1.f);
	ZoomPosition = ZoomCurve.GetRichCurveConst()->Eval(ZoomValue);
	
	const APawn* Pawn = GetPawn<APawn>();
	
	if (!Pawn)
	{
		return;
	}
	
	UCruorRTS_PlayerPawnMovementComponent* MovementComponent = Cast<UCruorRTS_PlayerPawnMovementComponent>(Pawn->GetMovementComponent());
	
	if (!MovementComponent)
	{
		return;
	}
	
	MovementComponent->MaxSpeed = FMath::Lerp(ZoomedOutMaxSpeed, ZoomedInMaxSpeed, ZoomPosition);
}

