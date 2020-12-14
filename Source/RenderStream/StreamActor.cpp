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
}

// Called every frame
void AStreamActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	bool ready = ((ARenderStreamGameModeBase*)this->gameMode)->GetInitState();
	if (ready && this->captureObj != nullptr) {
		this->CaptureFrame();
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
		FrameProcessData d(this->frameCounter, false, 0, nullptr, nullptr);
		if (this->FrameMap == nullptr || this->frameQueue == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("*Frame Queue or Frame Map is null."));
			return;
		}
		this->FrameMap->Add(this->frameCounter, &d);
		this->frameQueue->Enqueue(this->frameCounter);
		this->frameCounter++;
	}
}
