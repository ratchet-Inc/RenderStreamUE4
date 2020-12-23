// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamActor.h"
#include "RenderStreamGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "SocketSubsystem.h"
#include <fstream>

// Sets default values
AStreamActor::AStreamActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->FrameMap = new(std::nothrow) TMap<uint64_t, FrameProcessData*>();
	if (this->FrameMap == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to create the hash map."));
	}
	this->frameQueue = new(std::nothrow) TQueue<uint64_t>();
	if (this->frameQueue == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to create the frame queue."));
	}
	// network setup
	FString s("Stream socket for rendered frames.");
	//PLATFORM_SOCKETSUBSYSTEM for some reason i cant find the define
	//ISocketSubsystem* subsys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	ISocketSubsystem* subsys = ISocketSubsystem::Get(FName(L"WINDOWS"));
	this->socket = subsys->CreateSocket(EName::NAME_Stream, s, false);
	this->sharedRefAddr = subsys->CreateInternetAddr();
}

// Called when the game starts or when spawned
void AStreamActor::BeginPlay()
{
	Super::BeginPlay();
	this->gameMode = UGameplayStatics::GetGameMode(GetWorld());
#ifdef UE_BUILD_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("Stream Actor spawned."));
#endif
}

// Called every frame
void AStreamActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	bool ready = ((ARenderStreamGameModeBase*)this->gameMode)->GetInitState();
	if (ready && this->captureObj != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Capture"));
		this->CaptureFrame();
	}
}

void AStreamActor::ConnectToServer(void)
{
	bool reval = false;
	this->sharedRefAddr.Get()->SetIp(L"192.168.100.64", reval);
	if (!reval)
	{
		UE_LOG(LogTemp, Warning, L"*Failed to evaluate the IP address.");
		return;
	}
	this->sharedRefAddr.Get()->SetPort(8000);
	reval = this->socket->Connect(*this->sharedRefAddr);
	if (reval) {
		this->IsConnected = true;
	}
}

void AStreamActor::SetFrameGrabber(FFrameGrabber* pointer)
{
	this->captureObj = pointer;
}

void AStreamActor::CaptureFrame(void)
{
	FFramePayloadPtr pl;
	this->captureObj->CaptureThisFrame(pl);
	TArray<FCapturedFrameData> frames = this->captureObj->GetCapturedFrames();
	UINT l = frames.Num();
	UE_LOG(LogTemp, Warning, TEXT("Captured frames: %d."), l);
	if (l > 0) {
		FCapturedFrameData &frame = frames[l - 1];
		FrameProcessData* p = new(std::nothrow) FrameProcessData();
		FrameProcessData d(this->frameCounter, false, 0, 0, 0, nullptr, nullptr);
		if (this->FrameMap == nullptr || this->frameQueue == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("*Frame Queue or Frame Map is null."));
			return;
		}
		d.width = frame.BufferSize.X;
		d.height = frame.BufferSize.Y;
		p->height = d.height;
		p->width = d.width;
		d.frame = MakeShared<FCapturedFrameData*>(&frame);
		TSharedPtr<FrameProcessData*> ptr = MakeShared<FrameProcessData*>(&d);
		this->FrameMap->Add(this->frameCounter, p);
		this->frameQueue->Enqueue(this->frameCounter);
		this->frameCounter++;
		UE_LOG(LogTemp, Warning, TEXT("Frame saved: %d."), d.width*d.height);
	}
}

void AStreamActor::BeginDestroy(void)
{
	UE_LOG(LogTemp, Warning, TEXT("Destroy."));
	if (this->gameMode != nullptr) {
		((ARenderStreamGameModeBase*)this->gameMode)->ReleaseFrameGrabber();
#ifdef UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("released frame grabber."));
#endif
	}
	Super::BeginDestroy();
}

FrameProcessData* AStreamActor::FetchQueueData(uint64_t &frame_id)
{
	if (this->frameQueue != nullptr && !this->frameQueue->IsEmpty()) {
		bool res = this->frameQueue->Dequeue(frame_id);
		if (res == false) {
			return nullptr;
		}
		return *this->FrameMap->Find(frame_id);
	}
	return nullptr;
}

void AStreamActor::SendFrame(void)
{
	UE_LOG(LogTemp, Warning, TEXT("Send frame called."));
	bool reval = this->FrameMap->Contains(this->curSendCount);
	if (!reval) {
		UE_LOG(LogTemp, Warning, TEXT("Peek return: %d."), reval);
		TArray<uint64_t> keys;
		this->FrameMap->GetKeys(keys);
		uint64 min = -1;
		for(uint64 k : keys)
		{
			if (k <= min) {
				min = k;
			}
		}
		this->curSendCount = min;
		return;
	}
	/*FrameProcessData* d = *this->FrameMap->Find(this->curSendCount);
	if (!d->isReady) {
		UE_LOG(LogTemp, Warning, TEXT("frame not ready."));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Sending frame: %d."), this->curSendCount);
	int sent = 0, buffSize = d->arrLen;
	//this->sock->Send((uint8*)vals, buffSize, sent);
	UE_LOG(LogTemp, Warning, TEXT("bytes sent: %d."), sent);
	TCHAR recv[1024];
	//reval = this->sock->Recv((uint8*)recv, 1024, sent);
	if (!reval) {
		UE_LOG(LogTemp, Warning, TEXT("Socket failed."));
		this->IsConnected = false;
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("bytes read: %d."), sent);
	UE_LOG(LogTemp, Warning, TEXT("received: %s."), *FString(recv));
	this->FrameMap->Remove(this->curSendCount);
	this->curSendCount++;*/
}

/*
* Previous frame encoder to export raw uncompressed RGB to hex
*/
char* AStreamActor::ParseFrameFAST(const FCapturedFrameData* data, int& bufferSize, const int lossyInterval)
{
	if (data->ColorBuffer.Num() < 1) {
		return nullptr;
	}
	int block = 3 * 2;
	bufferSize = (data->BufferSize.X * data->BufferSize.Y * block) / lossyInterval;
	int counter = 0, counterLimit = data->ColorBuffer.Num();
	// a fail attempt at getting inner pixels within a specified resolution
	/*if (AStreamActor::frameSize < (data->BufferSize.X * data->BufferSize.Y)) {
		bufferSize = (AStreamActor::frameSize * block) / lossyInterval;
		counter = ((data->BufferSize.X * data->BufferSize.Y) / 2) - (AStreamActor::frameSize / 2);
		counterLimit = counter + AStreamActor::frameSize;
	}*/
	char* s = new (std::nothrow) char[bufferSize];
	if (s == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to allocated memory block."));
		return nullptr;
	}
	memset(s, 0, bufferSize);
	int index = 0, blankCounter = 0;
	static const char hex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	UE_LOG(LogTemp, Warning, TEXT("Frame Size: %s."), *data->BufferSize.ToString());
	std::ofstream fPtr;
	fPtr.open("C:/UE4_Dumps/UE4_Frame.txt");
	try {
		while (counter < counterLimit) {
			int rem = 0;
			int c = 0;
			int r = data->ColorBuffer[counter].R;
			int g = data->ColorBuffer[counter].G;
			int b = data->ColorBuffer[counter].B;
			if ((r | g | b) == 0) {
				blankCounter++;
			}
			while (r > 0) {
				rem = r % 16;
				fPtr << hex[rem];
				s[index + c] = hex[rem];
				r = r / 16;
				c++;
			}
			c++;
			while (g > 0) {
				rem = g % 16;
				fPtr << hex[rem];
				s[index + c] = hex[rem];
				g = g / 16;
				c++;
			}
			c++;
			while (b > 0) {
				rem = b % 16;
				fPtr << hex[rem];
				s[index + c] = hex[rem];
				b = b / 16;
				c++;
			}
			index += block;
			counter += lossyInterval;
		}
	}
	catch (...) {
		delete[] s;
		return nullptr;
	}
	fPtr.close();
	return s;
}
