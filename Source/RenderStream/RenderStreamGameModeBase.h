// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/GameEngine.h"
#include "FrameGrabber.h"
#include "EncoderThread.h"
#include "Includes/toojpeg.h"
#include "RenderStreamGameModeBase.generated.h"

struct EncoderThreadStructure {
	EncoderThread* enc = nullptr;
	FRunnableThread* thr = nullptr;
	explicit EncoderThreadStructure(EncoderThread* enc_, FRunnableThread* thr_) : enc(enc_), thr(thr_) {
	}
};

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
	static const uint8_t F_TIME_MS = 42;
	// JPEG compression quality
	static const uint8_t COMP_QUALITY = 60;

	ARenderStreamGameModeBase();
	virtual bool GetInitState(void);
	virtual uint8_t* ConvertFrame(TArray<FColor>& arr, unsigned int &len);
	virtual void ThreadOutPut(FString* msg) {
		mutex.Lock();
		UE_LOG(LogTemp, Warning, TEXT("%s"), msg);
		mutex.Unlock();
	}
	virtual void ReleaseFrameGrabber(void);
protected:
	virtual void BeginPlay(void) override;
private:
	bool startInit = false;
	uint8_t threadLimit = 1;
	TArray<EncoderThreadStructure> *Threads = nullptr;
	FCriticalSection mutex;
	TSharedPtr<FSceneViewport> scene_viewport;
	FFrameGrabber* capturePtr = nullptr;
	TArray<FFramePayloadPtr>* FramePayloadList = nullptr;
	AActor* streamActorPtr = nullptr;
	virtual void InitStream(void);
	virtual void InitFrameGrabber(void);
	virtual void GrabCurrentFrame(void);
	virtual void DetermineThreads(void);
	virtual void CreateEncoders(void);
};
