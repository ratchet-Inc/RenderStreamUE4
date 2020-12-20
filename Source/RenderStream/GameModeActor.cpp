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
	int res = 1;
	if (!isDone && ptr != nullptr) {
		switch (state) {
		case 0:
			((ARenderStreamGameModeBase*)ptr)->InitStream();
			state = 1;
			break;
		case 1:
			UE_LOG(LogTemp, Warning, TEXT("ticking."));
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
