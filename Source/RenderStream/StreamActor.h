// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrameGrabber.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "StreamActor.generated.h"

//#define DEBUG_FRAME_DUMP

class TooJPEG_Controller;

struct FrameProcessData {
	uint64_t f_id;
	bool isReady;
	unsigned short width;
	unsigned short height;
	unsigned int arrLen;
	uint8_t* arrRGB = nullptr;
	uint8_t* encoded = nullptr;
	TSharedPtr<FCapturedFrameData*, ESPMode::ThreadSafe> frame = nullptr;
	TooJPEG_Controller* encoder = nullptr;
	FrameProcessData() {
		f_id = 0;
		isReady = false;
		width = 0;
		height = 0;
		arrLen = 0;
		arrRGB = nullptr;
		encoded = nullptr;
		frame = nullptr;
		encoder = nullptr;
	}
	FrameProcessData(
		uint64_t id, bool r, uint16_t w, uint16_t h,
		UINT len, uint8_t* arr, uint8_t* en, TSharedPtr<FCapturedFrameData*, ESPMode::ThreadSafe> fm
	) : f_id(id), isReady(r), arrLen(len), arrRGB(arr), encoded(en) {
		width = w;
		height = h;
		fm = frame;
	}
};

UCLASS(Config = Game)
class RENDERSTREAM_API AStreamActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStreamActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay(void) override;
	virtual void BeginDestroy(void) override;
	//virtual void Destory(void) override;
	virtual void ConnectToServer(void);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetFrameGrabber(FFrameGrabber* pointer);
	virtual FrameProcessData* FetchQueueData(uint64_t &frame_id);
	virtual AGameModeBase* GetGameMode(void) { return this->gameMode;  }
private:
	UPROPERTY(Config)
		FString IPwide;
	double interval = 0.0f;
	TMap<uint64_t, FrameProcessData*>* FrameMap = nullptr;
	TQueue<uint64_t>* frameQueue = nullptr;
	AGameModeBase* gameMode = nullptr;
	FFrameGrabber* captureObj = nullptr;
	uint64_t frameCounter = 1;
	uint64_t curSendCount = 0;
	TSharedPtr<FInternetAddr> sharedRefAddr;
	FSocket* socket = nullptr;
	bool IsConnected = false;
	void CaptureFrame(void);
	void SendFrame(void);
	// legacy encoding, could still be useful
	char* ParseFrameFAST(const FCapturedFrameData* data, int& bufferSize, const int lossyInterval);
};
