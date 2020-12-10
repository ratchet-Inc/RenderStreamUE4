// Copyright Epic Games, Inc. All Rights Reserved.


#include "RenderStreamGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "StreamActor.h"
#include "FrameGrabber.h"
#include "toojpeg.h"
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
	UWorld* cWorld = GetWorld();
	APlayerController* pController = UGameplayStatics::GetPlayerController(cWorld, 0);
	UInputComponent* cInput = pController->InputComponent;
	cInput->BindKey(EKeys::Period, EInputEvent::IE_Pressed, this, &ARenderStreamGameModeBase::InitStream);
}

void ARenderStreamGameModeBase::InitStream(void)
{
	UE_LOG(LogTemp, Warning, TEXT("Key pressed."));
}

void ARenderStreamGameModeBase::InitFrameGrabber(void)
{
}

void ARenderStreamGameModeBase::GrabCurrentFrame(void)
{
}

void ARenderStreamGameModeBase::DetermineThreads(void)
{
}
