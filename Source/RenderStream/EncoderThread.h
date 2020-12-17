// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StreamActor.h"
#include "toojpeg.h"

/**
 * 
 */
class RENDERSTREAM_API EncoderThread : public FRunnable
{
public:
	EncoderThread(void);
	EncoderThread(const int tid, AStreamActor* master);
	virtual ~EncoderThread(void);

	virtual void SetDetails(const int threadID, AStreamActor* p) {
		this->thread_id = threadID;
		this->stream = p;
	};
	virtual void ForceStop(void) { this->forceStop = true; };

	// interface
	virtual bool Init(void);
	virtual uint32 Run(void);
	virtual void Stop(void);
private:
	int thread_id;
	bool forceStop = false;
	TooJPEG_Controller* encoder = nullptr;
	AStreamActor* stream = nullptr;
};
