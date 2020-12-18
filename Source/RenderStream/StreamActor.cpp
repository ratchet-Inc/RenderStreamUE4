// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamActor.h"
#include "RenderStreamGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"

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
		//this->CaptureFrame();
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
		FrameProcessData d(this->frameCounter, false, 0, 0, 0, nullptr, nullptr);
		if (this->FrameMap == nullptr || this->frameQueue == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("*Frame Queue or Frame Map is null."));
			return;
		}
		d.width = frame.BufferSize.X;
		d.height = frame.BufferSize.Y;
		d.frame = &frame;
		this->FrameMap->Add(this->frameCounter, &d);
		this->frameQueue->Enqueue(this->frameCounter);
		this->frameCounter++;
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

uint64_t AStreamActor::FetchQueueData(FrameProcessData* memory)
{
	if (this->frameQueue != nullptr && !this->frameQueue->IsEmpty()) {
		unsigned long long val = 0;
		bool res = this->frameQueue->Dequeue(val);
		if (res == false) {
			return 0;
		}
		memory = *(this->FrameMap->Find(val));
		return val;
	}
	return 0;
}
