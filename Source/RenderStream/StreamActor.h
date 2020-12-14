// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrameGrabber.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "StreamActor.generated.h"

struct FrameProcessData {
	uint64_t f_id;
	bool isReady;
	unsigned int arrLen;
	uint8_t* arrRGB = nullptr;
	uint8_t* encoded = nullptr;
	explicit FrameProcessData(
		uint64_t id, bool r, UINT len,
		uint8_t* arr, uint8_t* en
	) : f_id(id), isReady(r), arrLen(len), arrRGB(arr), encoded(en) {
	}
};

UCLASS()
class RENDERSTREAM_API AStreamActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStreamActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetFrameGrabber(FFrameGrabber* pointer);
private:
	TMap<uint64_t, FrameProcessData*>* FrameMap = nullptr;
	TQueue<uint64_t>* frameQueue = nullptr;
	AGameModeBase* gameMode = nullptr;
	FFrameGrabber* captureObj = nullptr;
	uint64_t frameCounter = 0;
	uint64_t curframeCount = 0;
	virtual void CaptureFrame(void);
};
