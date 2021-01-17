// Fill out your copyright notice in the Description page of Project Settings.


#include "EncoderThread.h"
#include "RenderStreamGameModeBase.h"

EncoderThread::EncoderThread(void)
{
	this->thread_id = -1;
}

EncoderThread::EncoderThread(const int tid, AStreamActor* master) : thread_id(tid), stream(master)
{
}

EncoderThread::~EncoderThread()
{
	if (this->encoder != nullptr) {
		this->encoder->EmptyMemoryStore();
		delete this->encoder;
	}
}

bool EncoderThread::Init(void)
{
	this->encoder = new(std::nothrow) TooJPEG_Controller();
	return true;
}

uint32 EncoderThread::Run(void)
{
	float sleepTime = ((float)ARenderStreamGameModeBase::FPS_LIMIT) / 1000.0f;
	UE_LOG(LogTemp, Warning, TEXT("thread ready, sleep time: %f."), sleepTime);
	while (true) {
		if (this->forceStop) {
			break;
		}
		if (this->stream == nullptr || this->encoder == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("thread failure."));
			return 1;
		}
		unsigned int x = 0;
		this->encoder->GetEncoded(x);
		// checking if the stream buffer was emptied by the main thread.
		if (x == 0) {
			ARenderStreamGameModeBase* gm = (ARenderStreamGameModeBase*)this->stream->GetGameMode();
			/*FString m("thread running.");
			gm->ThreadOutPut(m);*/
			unsigned long long frameID = 0;
			FrameProcessData* ptr = this->stream->FetchQueueData(frameID);
			if (frameID == 0 || ptr == nullptr) {
				FPlatformProcess::Sleep(0.03f);
				continue;
			}
			//UE_LOG(LogTemp, Warning, L"encoding...");
			if (ptr->arrRGB == nullptr || ptr->arrLen == 0) {
				UE_LOG(LogTemp, Error, L"frame is null.");
				continue;
			}
			bool res = this->encoder->EncodeImage(ptr->arrRGB, ptr->width, ptr->height, true, ARenderStreamGameModeBase::COMP_QUALITY);
			ptr->isReady = res;
			if (!res) {
				FString s("Failed to encode frame[");
				s += frameID + "].";
				gm->ThreadOutPut(s);
			}
			ptr->encoded = this->encoder->GetEncoded(ptr->arrLen);
			ptr->encoder = this->encoder;
#ifdef UE_BUILD_DEBUG
			UE_LOG(LogTemp, Error, TEXT("frame encoded[%d]."), frameID);
#endif
		}
		FPlatformProcess::Sleep(0.03f);
	}

	return 0;
}

void EncoderThread::Stop(void)
{
	UE_LOG(LogTemp, Warning, TEXT("stopping thread."));
	this->forceStop = true;
}
