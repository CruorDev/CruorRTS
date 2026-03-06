//  © Copyright Cruor ehf.  2026  All Rights Reserved.


#include "CruorRTSCoreGameplayTags.h"


namespace CruorRTSCoreGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.CruorRTSPlayerPawn.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Spin, "InputTag.CruorRTSPlayerPawn.Spin", "Spin input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact, "InputTag.CruorRTSPlayerPawn.Interact", "Interaction input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Spin_Started, "InputTag.CruorRTSPlayerPawn.Spin.Started", "Spin input started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Zoom, "InputTag.CruorRTSPlayerPawn.Zoom", "Zoom input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Zoom_Started, "InputTag.CruorRTSPlayerPawn.Zoom.Started", "Zoom input started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_EnableDragMove_Started, "InputTag.CruorRTSPlayerPawn.EnableDragMove.Started", "EnableDragMove input started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_EnableDragMove_Canceled, "InputTag.CruorRTSPlayerPawn.EnableDragMove.Canceled", "EnableDragMove input canceled.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_EnableDragMove_Completed, "InputTag.CruorRTSPlayerPawn.EnableDragMove.Completed", "EnableDragMove input completed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_DragMove, "InputTag.CruorRTSPlayerPawn.DragMove", "DragMove input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_MouseZoomSpin, "InputTag.CruorRTSPlayerPawn.MouseZoomSpin", "Mouse Zoom and Spin with the same input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_TouchZoomAndSpin, "InputTag.CruorRTSPlayerPawn.TouchZoomAndSpin", "Touch Zoom and Spin with the same input.");
}
