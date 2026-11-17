#include "ERRoom.h"

#include "ERCommon.h"
#include "Components/SceneComponent.h"

AERRoom::AERRoom()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;
}

void AERRoom::Build()
{
	const FLinearColor Floor(0.22f, 0.15f, 0.10f);
	const FLinearColor Wall(0.50f, 0.48f, 0.43f);
	const FLinearColor Ceil(0.60f, 0.60f, 0.58f);
	const FLinearColor Trim(0.16f, 0.09f, 0.05f);
	const FLinearColor Wood(0.32f, 0.19f, 0.09f);
	USceneComponent* R = RootComponent;

	// ---- 部屋 (内寸 800 x 800 x 300)
	ER::AddBox(this, R, FVector(0, 0, -10), FVector(820, 820, 20), Floor);
	ER::AddBox(this, R, FVector(0, 0, 310), FVector(820, 820, 20), Ceil);
	ER::AddBox(this, R, FVector(0, -410, 150), FVector(840, 20, 300), Wall);
	ER::AddBox(this, R, FVector(410, 0, 150), FVector(20, 840, 300), Wall);
	ER::AddBox(this, R, FVector(-410, 0, 150), FVector(20, 840, 300), Wall);
	// 北の壁（扉の開口部 幅120 x 高さ220）
	ER::AddBox(this, R, FVector(-240, 410, 150), FVector(360, 20, 300), Wall);
	ER::AddBox(this, R, FVector(240, 410, 150), FVector(360, 20, 300), Wall);
	ER::AddBox(this, R, FVector(0, 410, 260), FVector(120, 20, 80), Wall);
	// 扉枠
	ER::AddBox(this, R, FVector(-64, 402, 112), FVector(8, 24, 224), Trim);
	ER::AddBox(this, R, FVector(64, 402, 112), FVector(8, 24, 224), Trim);
	ER::AddBox(this, R, FVector(0, 402, 224), FVector(136, 24, 8), Trim);
	// 幅木
	ER::AddBox(this, R, FVector(0, -398, 5), FVector(800, 4, 10), Trim, FRotator::ZeroRotator, false);
	ER::AddBox(this, R, FVector(398, 0, 5), FVector(4, 800, 10), Trim, FRotator::ZeroRotator, false);
	ER::AddBox(this, R, FVector(-398, 0, 5), FVector(4, 800, 10), Trim, FRotator::ZeroRotator, false);

	// ---- 出口の廊下
	ER::AddBox(this, R, FVector(0, 575, -10), FVector(200, 310, 20), Floor);
	ER::AddBox(this, R, FVector(0, 575, 310), FVector(200, 310, 20), Ceil);
	ER::AddBox(this, R, FVector(-90, 575, 150), FVector(20, 310, 300), Wall);
	ER::AddBox(this, R, FVector(90, 575, 150), FVector(20, 310, 300), Wall);
	ER::AddBox(this, R, FVector(0, 740, 150), FVector(200, 20, 300), FLinearColor(0.85f, 0.9f, 0.85f));
	ER::AddText(this, R, FVector(0, 728, 190), TEXT("EXIT"), 50, FColor(40, 255, 80), FRotator(0, -90, 0));

	// ---- 家具: 机と椅子（東の壁）
	ER::AddBox(this, R, FVector(330, -150, 74), FVector(70, 140, 5), Wood);
	for (const FVector2D& Leg : { FVector2D(300, -215), FVector2D(360, -215), FVector2D(300, -85), FVector2D(360, -85) })
	{
		ER::AddBox(this, R, FVector(Leg.X, Leg.Y, 36), FVector(5, 5, 72), Wood);
	}
	ER::AddBox(this, R, FVector(255, -150, 45), FVector(40, 40, 5), Wood);
	ER::AddBox(this, R, FVector(237, -150, 72), FVector(4, 40, 50), Wood);
	for (const FVector2D& Leg : { FVector2D(238, -168), FVector2D(272, -168), FVector2D(238, -132), FVector2D(272, -132) })
	{
		ER::AddBox(this, R, FVector(Leg.X, Leg.Y, 21), FVector(4, 4, 42), Wood);
	}
	// 本棚（飾り）: 西の壁
	ER::AddBox(this, R, FVector(-385, -280, 90), FVector(30, 100, 180), Wood);
	const FLinearColor Books[] = { FLinearColor(0.4f, 0.1f, 0.1f), FLinearColor(0.1f, 0.2f, 0.3f), FLinearColor(0.3f, 0.3f, 0.25f), FLinearColor(0.15f, 0.25f, 0.15f) };
	for (int32 Shelf = 0; Shelf < 3; ++Shelf)
	{
		for (int32 b = 0; b < 8; ++b)
		{
			ER::AddBox(this, R, FVector(-368, -320 + b * 11, 30 + Shelf * 55), FVector(4, 9, 28 + (b % 3) * 4), Books[(b + Shelf) % 4], FRotator::ZeroRotator, false);
		}
	}
	// 床のラグ・天井のランプ
	ER::AddBox(this, R, FVector(0, 0, 0.5f), FVector(300, 220, 1), FLinearColor(0.35f, 0.08f, 0.07f), FRotator::ZeroRotator, false);
	ER::AddSphere(this, R, FVector(0, 0, 292), 24, FLinearColor(1.f, 0.95f, 0.8f));

	// ---- 照明: 影付きは1灯だけ。残りは影なしの補助光
	ER::AddPointLight(this, R, FVector(0, 0, 265), 4000.f, 1100.f, FLinearColor(1.f, 0.88f, 0.72f), true);
	ER::AddPointLight(this, R, FVector(0, -150, 140), 1200.f, 900.f, FLinearColor(0.8f, 0.85f, 1.f), false);
	ER::AddPointLight(this, R, FVector(0, 600, 250), 2500.f, 500.f, FLinearColor(0.8f, 1.f, 0.85f), false);
}
