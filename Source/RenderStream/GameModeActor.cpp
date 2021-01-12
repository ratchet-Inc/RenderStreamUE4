// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeActor.h"
#include "RenderStreamGameModeBase.h"

// Sets default values
AGameModeActor::AGameModeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameModeActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("auto init value: %d."), this->autoInit);
}

void AGameModeActor::BeginDestroy(void)
{
	if (ptr != NULL) {
		((ARenderStreamGameModeBase*)ptr)->ReleaseEncoders();
	}
	Super::BeginDestroy();
}

// Called every frame
void AGameModeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->waitTimer += DeltaTime;
	if (this->waitTimer < 1.0f) {
		return;
	}
	int res = 1;
	if (!isDone && ptr != nullptr) {
		switch (state) {
		case 0:
			res = ((ARenderStreamGameModeBase*)ptr)->InitStream_Main();
			// trying to equate to 1, if the returned values = 0
			// its successful, but we are adding 1 to avoid an else statement
			res++;
			if (res != 1) {
				res = 0;
			}
			state = res;
			res = 1;
			break;
		case 1:
			res = ((ARenderStreamGameModeBase*)ptr)->DetermineThreads();
			if (res == -1) {
				isDone = true;
			}
			else if (res == 0) {
				state = 2;
			}
			break;
		case 2:
			UE_LOG(LogTemp, Warning, TEXT("started."));
			((ARenderStreamGameModeBase*)ptr)->StartStream();
			isDone = true;
			break;
		}
	}
}

void AGameModeActor::SetGameMode(AGameModeBase* pointer)
{
	this->ptr = pointer;
}
