//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "PlayerPawn/CruorRTS_PlayerPawnMovementComponent.h"
#include "CommonInputTypeEnum.h"
#include "PlayerPawn/CruorRTS_PlayerPawn.h"
#include "Components/SphereComponent.h"
#include "CruorRTS/CruorRTSLogChannels.h"

#define COLLISION_LANDSCAPE ECC_GameTraceChannel6

namespace CruorRTSPlayerPawn
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(
		TEXT("CruorRTSPlayerPawn.GroundTraceDistance"), 
		GroundTraceDistance, 
		TEXT("Distance to trace up when updating the RootComponents's z-height."), 
		ECVF_Cheat);
}

UCruorRTS_PlayerPawnMovementComponent::UCruorRTS_PlayerPawnMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UCruorRTS_PlayerPawnMovementComponent::TickComponent(
	float DeltaTime, 
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	MoveTracking();
	UpdatePivotPoint();
}

void UCruorRTS_PlayerPawnMovementComponent::MoveTracking()
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		const ACruorRTS_PlayerPawn* CommanderPawn = Cast<ACruorRTS_PlayerPawn>(GetPawnOwner());
		USceneComponent* Selector = CommanderPawn->GetSelector();
		float ScreenLocationX;
		float ScreenLocationY;
		int32 ScreenSizeX;
		int32 ScreenSizeY;
		PlayerController->GetViewportSize(ScreenSizeX, ScreenSizeY);
		const float CenterScreenX = static_cast<float>(ScreenSizeX / 2);
		const float CenterScreenY = static_cast<float>(ScreenSizeY / 2);
		FHitResult HitResultSelector;

		switch (CommanderPawn->GetCurrentInputType())
		{
		case ECommonInputType::Gamepad:
			PlayerController->GetHitResultAtScreenPosition(
				FVector2D(CenterScreenX, CenterScreenY), 
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);
			Selector->SetWorldLocation(HitResultSelector.Location);
			UpdateCursorProjection();
			break;
		case ECommonInputType::MouseAndKeyboard:
			if (PlayerController->GetMousePosition(ScreenLocationX, ScreenLocationY))
			{
				PlayerController->GetHitResultUnderCursor(
					ECollisionChannel::COLLISION_LANDSCAPE, 
					false, 
					HitResultSelector);
				Selector->SetWorldLocation(HitResultSelector.Location);
				UpdateCursorProjection();
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, FString::Printf(TEXT("Selection: %s"), *Selector->GetComponentLocation().ToString()));
				break;
			}
			PlayerController->GetHitResultAtScreenPosition(
				FVector2D(CenterScreenX, CenterScreenY), 
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);	
			Selector->SetWorldLocation(HitResultSelector.Location);
			UpdateCursorProjection();
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
				Selector->SetWorldLocation(HitResultSelector.Location);
				UpdateCursorProjection();
				ScreenLocationIntersection = HitResultSelector.Location;
				break;
			}
			Selector->SetWorldLocation(ScreenLocationIntersection + FVector(0.f, 0.f, -500.f));
			break;
		default:
			PlayerController->GetHitResultAtScreenPosition(FVector2D(
				CenterScreenX, CenterScreenY), 
				ECollisionChannel::COLLISION_LANDSCAPE, 
				false, 
				HitResultSelector);
			Selector->SetWorldLocation(HitResultSelector.Location);
			UpdateCursorProjection();
			break;
		}
	}
}

void UCruorRTS_PlayerPawnMovementComponent::UpdatePivotPoint() const
{
	if (const UWorld* World = GetWorld())
	{
		const ACruorRTS_PlayerPawn* CommanderPawn = Cast<ACruorRTS_PlayerPawn>(GetPawnOwner());
		FHitResult HitResultAboveRoot;
		World->LineTraceSingleByChannel(HitResultAboveRoot,
			CommanderPawn->GetRootComponent()->GetComponentLocation() 
			+ CommanderPawn->GetRootComponent()->GetUpVector() 
			* CruorRTSPlayerPawn::GroundTraceDistance,
			CommanderPawn->GetRootComponent()->GetComponentLocation() 
			+ CommanderPawn->GetRootComponent()->GetUpVector() 
			* -CruorRTSPlayerPawn::GroundTraceDistance,
			ECollisionChannel::COLLISION_LANDSCAPE);
		HitResultAboveRoot.bBlockingHit ? 
		CommanderPawn->GetPivotPoint()->SetRelativeLocation(FVector(0.f, 0.f, HitResultAboveRoot.Location.Z)) 
		: CommanderPawn->GetPivotPoint()->SetRelativeLocation(FVector::ZeroVector);
	}
}

void UCruorRTS_PlayerPawnMovementComponent::UpdateCursorProjection() const
{
	const ACruorRTS_PlayerPawn* CommanderPawn = Cast<ACruorRTS_PlayerPawn>(GetPawnOwner());
	const USceneComponent* RootSceneComponent = CommanderPawn->GetRootComponent();
	const USceneComponent* Selector = CommanderPawn->GetSelector();
	UStaticMeshComponent* CursorProjection = CommanderPawn->GetCursorProjection();
	
	if (const UWorld* World = GetWorld())
	{
		CursorProjection->SetWorldLocation(
			FMath::VInterpTo(CursorProjection->GetComponentLocation(), 
				Selector->GetComponentLocation(), 
				World->GetDeltaSeconds(), 12.f));
		FHitResult HitResultCursor;
		World->LineTraceSingleByChannel(HitResultCursor,
			CursorProjection->GetComponentLocation() + 
			Selector->GetUpVector() * CruorRTSPlayerPawn::GroundTraceDistance,
			CursorProjection->GetComponentLocation() + 
			Selector->GetUpVector() * -CruorRTSPlayerPawn::GroundTraceDistance,
			ECollisionChannel::COLLISION_LANDSCAPE);
		CursorProjection->SetWorldTransform(FTransform(
			FRotationMatrix::MakeFromZ(HitResultCursor.Normal).Rotator(), 
			FVector(CursorProjection->GetComponentLocation().X, 
				CursorProjection->GetComponentLocation().Y, 
				HitResultCursor.Location.Z + 15.f), 
				CursorProjection->GetComponentScale()));
		// *If CursorProjection isn't circular this keeps the top always pointing up
		CursorProjection->AddLocalRotation(
			FRotator(0.f, FMath::Wrap(static_cast<float>(RootSceneComponent->GetComponentRotation().Yaw) 
				+ 90.f, -180.f, 180.f) - 
				CursorProjection->GetComponentRotation().Yaw, 0.f));
		
		
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT("CursorProjection: %s"), *CursorProjection->GetComponentLocation().ToString()));
	}
}

#undef COLLISION_LANDSCAPE


