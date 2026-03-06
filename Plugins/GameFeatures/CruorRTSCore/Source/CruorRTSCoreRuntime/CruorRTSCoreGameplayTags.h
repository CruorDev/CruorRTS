//  © Copyright Cruor ehf.  2026  All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace CruorRTSCoreGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Spin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Spin_Started);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Zoom);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Zoom_Started);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EnableDragMove_Started);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EnableDragMove_Canceled);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_EnableDragMove_Completed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_DragMove);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_MouseZoomSpin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_TouchZoomAndSpin);
}
