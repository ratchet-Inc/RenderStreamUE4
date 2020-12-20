// Copyright Epic Games, Inc. All Rights Reserved.


#include "RenderStreamGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "time.h"
#include "StreamActor.h"
#include "GameModeActor.h"
#include "FrameGrabber.h"
//#include "toojpeg.cpp"
#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "IAssetViewport.h"
#endif

ARenderStreamGameModeBase::ARenderStreamGameModeBase(void) : Super()
{
	this->FramePayloadList = new TArray<FFramePayloadPtr>();
}

void ARenderStreamGameModeBase::BeginPlay(void)
{
	this->streamActorPtr = GetWorld()->SpawnActor<AStreamActor>();
	this->tickObj = GetWorld()->SpawnActor<AGameModeActor>();
	if (this->tickObj != nullptr) {
		((AGameModeActor*)this->tickObj)->SetGameMode((AGameModeBase*)this);
	}
	UWorld* cWorld = GetWorld();
	APlayerController* pController = UGameplayStatics::GetPlayerController(cWorld, 0);
	UInputComponent* cInput = pController->InputComponent;
	cInput->BindKey(EKeys::Period, EInputEvent::IE_Pressed, this, &ARenderStreamGameModeBase::InitStream);
}

bool ARenderStreamGameModeBase::GetInitState(void)
{
	bool val = this->startInit;
	this->startInit = false;
	return val;
}

uint8_t* ARenderStreamGameModeBase::ConvertFrame(TArray<FColor>& arr, unsigned int &len)
{
	UINT index = 0;
	len = arr.Num() * 3;
	unsigned char* memBuffer = new(std::nothrow) unsigned char[len];
	for (int i = 0; i < arr.Num(); ++i) {
		memBuffer[index + 0] = arr[i].R;
		memBuffer[index + 1] = arr[i].G;
		memBuffer[index + 2] = arr[i].B;
		index += 3;
	}
	return memBuffer;
}

void ARenderStreamGameModeBase::InitStream(void)
{
#ifdef UE_BUILD_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("Manually starting stream."));
#endif
	//this->startInit = true;
	this->InitFrameGrabber();
	if (this->streamActorPtr == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("*Failed to spawn stream actor."));
		return;
	}
	this->DetermineThreads();
	((AStreamActor*)this->streamActorPtr)->SetFrameGrabber(this->capturePtr);

	return;
}

void ARenderStreamGameModeBase::InitFrameGrabber(void)
{
	TSharedPtr<FSceneViewport> sceneVP;
	if (this->capturePtr == nullptr) {
#if WITH_EDITOR
		if (GIsEditor) {
			for (const FWorldContext& Context : GEngine->GetWorldContexts()) {
				if (Context.WorldType == EWorldType::PIE) {
					FSlatePlayInEditorInfo* sess = GEditor->SlatePlayInEditorMap.Find(Context.ContextHandle);
					if (sess && sess->DestinationSlateViewport.IsValid()) {
						TSharedPtr<IAssetViewport> destVP = sess->DestinationSlateViewport.Pin();
						sceneVP = destVP.Get()->GetSharedActiveViewport();
					}
					if (sess && sess->SlatePlayInEditorWindowViewport.IsValid()) {
						sceneVP = sess->SlatePlayInEditorWindowViewport;
					}
					if (/*this->scene_viewport.IsValid()*/sceneVP.IsValid()) {
						break;
					}
				}
			}
		}
#else
		{
			UGameEngine* en = Cast<UGameEngine>(GEngine);
			this->scene_viewport = en->SceneViewport;
		}
#endif
		// cannot save viewport in class instance, UE will assert on exit.
		// need to find a way to void the refernce pointer or decrease count.
		//this->scene_viewport = MakeShared<FSceneViewport>(sceneVP);
		this->capturePtr = new(std::nothrow) FFrameGrabber(sceneVP.ToSharedRef(), sceneVP.Get()->GetSize());
		if (this->capturePtr == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("*Failed to create screen capture object."));
			return;
		}
		this->capturePtr->StartCapturingFrames();
	}
}

void ARenderStreamGameModeBase::GrabCurrentFrame(void)
{
}

int ARenderStreamGameModeBase::DetermineThreads(void)
{
	if (this->capturePtr == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("*Frame Grabber is null."));
		return 1;
	}
	FFramePayloadPtr payl;
	this->capturePtr->CaptureThisFrame(payl);
	TArray<FCapturedFrameData> frameData = this->capturePtr->GetCapturedFrames();
	if (frameData.Num() < 1) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to get any rendered frames."));
		return 1;
	}
	FCapturedFrameData& data = frameData.Last();
	unsigned int memLen = 0;
	uint8_t* memory = this->ConvertFrame(data.ColorBuffer, memLen);
	TooJPEG_Controller* contr = new(std::nothrow) TooJPEG_Controller();
	if (contr == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to construct the jpeg controller."));
		return 1;
	}
	int frameWidth = data.BufferSize.X;
	int frameHeight = data.BufferSize.Y;
	unsigned char q = ARenderStreamGameModeBase::COMP_QUALITY;
	time_t start = clock();
	contr->EncodeImage(memory, frameWidth, frameHeight, true, q);
	time_t end = clock();
	contr->EmptyMemoryStore();
	delete contr;
	double timeTaken = (double)(end - start) / CLOCKS_PER_SEC;
	UE_LOG(LogTemp, Warning, TEXT("Estimated conversion time: %.6f."), timeTaken);
	uint8_t coreCount = timeTaken / ARenderStreamGameModeBase::F_TIME_MS;
	coreCount++;
	this->threadLimit = coreCount;
	UE_LOG(LogTemp, Warning, TEXT("Encoder threads(determined): %d."), coreCount);
	int r = this->CreateEncoders();
	if (r != 0) {
		r = -1;
	}
	return r;
}

void ARenderStreamGameModeBase::ReleaseEncoders(void)
{
	if (this->Threads != nullptr) {
		EncoderThreadStructure* p = this->Threads->GetData();
		for (int i = 0; i < this->Threads->Num(); ++i) {
			delete p[i].thr;
			delete p[i].enc;
		}
	}
}

void ARenderStreamGameModeBase::ReleaseFrameGrabber(void)
{
	if (this->capturePtr != nullptr) {
		this->capturePtr->StopCapturingFrames();
		this->capturePtr->Shutdown();
		this->scene_viewport.Reset();
		delete this->capturePtr;
	}
}

int ARenderStreamGameModeBase::CreateEncoders(void)
{
	this->Threads = new(std::nothrow) TArray<EncoderThreadStructure>();
	if (this->Threads == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("*Failed to create encoding threads vector[%d]."), this->threadLimit);
		return 1;
	}
	for (int i = 0; i < this->threadLimit; ++i) {
		EncoderThread* ptr = new(std::nothrow) EncoderThread();
		if (ptr == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("*Failed to create an encoder."));
			return 1;
		}
		ptr->SetDetails(i, (AStreamActor*)this->streamActorPtr);
		EncoderThreadStructure s(
			ptr,
			FRunnableThread::Create(ptr, TEXT("a JPEG encoder thread."))
		);
		this->Threads->Add(s);
	}
	UE_LOG(LogTemp, Warning, TEXT("Threads created."));

	return 0;
}
