// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamActor.h"

// Sets default values
AStreamActor::AStreamActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStreamActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStreamActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

