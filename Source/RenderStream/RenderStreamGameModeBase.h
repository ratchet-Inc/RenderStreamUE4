// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/GameEngine.h"
#include "FrameGrabber.h"
#include "RenderStreamGameModeBase.generated.h"

class TooJpeg_Controller;

/**
 * 
 */
UCLASS()
class RENDERSTREAM_API ARenderStreamGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	// frames per second cap
	static const uint8_t FPS_LIMIT = 24;
	// frame time in milliseconds
	static const uint8_t F_TIME_MS = 41;

	ARenderStreamGameModeBase();
protected:
	virtual void BeginPlay(void) override;
private:
	uint8_t threadLimit = 1;
	FRunnableThread* Threads = nullptr;
	FCriticalSection mutex;
	TSharedPtr<FSceneViewport>* scene_viewport;
	TSharedRef<FSceneViewport>* sceneRef;
	FFrameGrabber* capturePtr = nullptr;
	TArray<FFramePayloadPtr>* FramePayloadList = nullptr;
	AActor* streamActorPtr = nullptr;
	virtual void InitStream(void);
	virtual void InitFrameGrabber(void);
	virtual void GrabCurrentFrame(void);
	virtual void DetermineThreads(void);
};
