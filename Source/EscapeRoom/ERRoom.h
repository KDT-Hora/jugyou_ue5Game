#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ERRoom.generated.h"

/** 床・壁・天井・家具・照明をまとめた静的アクター（Tick なし・Static モビリティ） */
UCLASS()
class AERRoom : public AActor
{
	GENERATED_BODY()

public:
	AERRoom();
	void Build();
};
