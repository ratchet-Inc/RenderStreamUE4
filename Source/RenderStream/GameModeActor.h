// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameModeActor.generated.h"

UCLASS(Config=Game)
class RENDERSTREAM_API AGameModeActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AGameModeActor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetGameMode(AGameModeBase* pointer);
	virtual int GetAutoInit(void) { return this->autoInit; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy(void) override;
private:
	UPROPERTY(Config)
		int autoInit;
	AGameModeBase* ptr = nullptr;
	bool isDone = false;
	int state = 0;
	float waitTimer = 0.0f;
};
