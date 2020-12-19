// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeActor.h"

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

// Called every frame
void AGameModeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameModeActor::SetGameMode(AGameModeBase* pointer)
{
	this->ptr = pointer;
}
