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
	if (this->forceStop) {
		return 0;
	}
	if (this->stream == nullptr || this->encoder == nullptr) {
		return 1;
	}
	FrameProcessData* ptr = nullptr;
	unsigned long long frameID = this->stream->FetchQueueData(ptr);
	if (frameID == 0 || ptr == nullptr) {
		FPlatformProcess::Sleep(ARenderStreamGameModeBase::FPS_LIMIT / 1000);
		return 1;
	}
	ARenderStreamGameModeBase* gm = (ARenderStreamGameModeBase*)this->stream->GetGameMode();
	ptr->arrRGB = gm->ConvertFrame(ptr->frame->ColorBuffer, ptr->arrLen);
	bool res = this->encoder->EncodeImage(ptr->arrRGB, ptr->width, ptr->height, true, ARenderStreamGameModeBase::COMP_QUALITY);
	ptr->isReady = res;
	if (!res) {
		FString s("Failed to encode frame.");
		gm->ThreadOutPut(&s);
	}
	return 0;
}

void EncoderThread::Stop(void)
{
}
