// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrameGrabber.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "StreamActor.generated.h"

struct FrameProcessData {
	uint64_t f_id;
	bool isReady;
	unsigned short width;
	unsigned short height;
	unsigned int arrLen;
	uint8_t* arrRGB = nullptr;
	uint8_t* encoded = nullptr;
	FCapturedFrameData* frame = nullptr;
	explicit FrameProcessData(
		uint64_t id, bool r, uint16_t w, uint16_t h,
		UINT len, uint8_t* arr, uint8_t* en
	) : f_id(id), isReady(r), arrLen(len), arrRGB(arr), encoded(en) {
		width = w;
		height = h;
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
	virtual void BeginPlay(void) override;
	virtual void BeginDestroy(void) override;
	//virtual void Destory(void) override;
	virtual void ConnectToServer(void);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetFrameGrabber(FFrameGrabber* pointer);
	virtual uint64_t FetchQueueData(FrameProcessData* memory);
	virtual AGameModeBase* GetGameMode(void) { return this->gameMode;  }
private:
	TMap<uint64_t, FrameProcessData*>* FrameMap = nullptr;
	TQueue<uint64_t>* frameQueue = nullptr;
	AGameModeBase* gameMode = nullptr;
	FFrameGrabber* captureObj = nullptr;
	uint64_t frameCounter = 0;
	uint64_t curSendCount = 0;
	TSharedPtr<FInternetAddr> sharedRefAddr;
	FSocket* socket = nullptr;
	bool IsConnected = false;
	void CaptureFrame(void);
	void SendFrame(void);
	// legacy encoding, could still be useful
	char* ParseFrameFAST(const FCapturedFrameData* data, int& bufferSize, const int lossyInterval);
};
