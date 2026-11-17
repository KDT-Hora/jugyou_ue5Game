#include "ERGameMode.h"

#include "ERCharacter.h"
#include "ERCommon.h"
#include "ERHUD.h"
#include "ERProps.h"
#include "ERRoom.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AERGameMode::AERGameMode()
{
	DefaultPawnClass = AERCharacter::StaticClass();
	HUDClass = AERHUD::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

template <class T>
T* AERGameMode::SpawnAt(const FVector& Loc, float Yaw)
{
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<T>(T::StaticClass(), Loc, FRotator(0.f, Yaw, 0.f), P);
}

void AERGameMode::StartPlay()
{
	BuildLevel();
	Super::StartPlay();

	Objective = ERT("部屋を調べて、脱出の手がかりを探そう");
	ShowMessage(ERT("目が覚めると、見知らぬ部屋に閉じ込められていた…。"), 5.f);
}

void AERGameMode::RestartPlayer(AController* NewPlayer)
{
	// PlayerStart を置かずに固定位置へスポーン（扉の方を向く）
	RestartPlayerAtTransform(NewPlayer, FTransform(FRotator(0.f, 90.f, 0.f), FVector(0.f, -250.f, 100.f)));
}

void AERGameMode::BuildLevel()
{
	// 部屋（床・壁・家具・照明）
	SpawnAt<AERRoom>(FVector::ZeroVector, 0.f)->Build();

	// ① 机の上の日誌 → 時計がヒント
	AERNote* Diary = SpawnAt<AERNote>(FVector(320.f, -170.f, 76.5f), 180.f);
	Diary->Init(ERT("研究日誌"),
		ERT("…あの日から、この部屋の時間は止まったままだ。\n\n"
			"大切な鍵は金庫にしまっておいた。\n"
			"番号は、止まった時計が覚えている。\n\n"
			"（「時」→「分」の順に3桁）"),
		ERT("止まった時計を探して、金庫の番号を突き止めよう"));

	// ② 止まった時計 3:40 → 金庫の番号 3-4-0
	SpawnAt<AERInspect>(FVector(-400.f, -100.f, 200.f), 0.f)->BuildClock(3, 40);

	// ③ 金庫（中に鍵）
	AERPickup* Key = SpawnAt<AERPickup>(FVector(-200.f, -365.f, 5.f), 0.f);
	Key->Init(TEXT("Key"), ERT("古い鍵"));
	SpawnAt<AERSafe>(FVector(-200.f, -400.f, 0.f), 90.f)->Build({ 3, 4, 0 }, Key);

	// ④ 戸棚（鍵で開く・中にメモ）
	AERNote* Memo = SpawnAt<AERNote>(FVector(375.f, 150.f, 87.5f), 180.f);
	Memo->Init(ERT("走り書きのメモ"),
		ERT("扉の暗証番号は4桁。\n\n"
			"この部屋の絵に描かれた数字を\n"
			"　赤 → 青 → 緑 → 黄\n"
			"の順に並べること。"),
		ERT("色の絵を探して、扉の暗証番号を解こう"));
	SpawnAt<AERCabinet>(FVector(400.f, 150.f, 0.f), 180.f)->Build(TEXT("Key"), Memo);

	// ⑤ 4枚の色付きの絵 → 暗証番号 7295
	SpawnAt<AERInspect>(FVector(150.f, -400.f, 160.f), 90.f)->BuildPainting(FLinearColor(0.75f, 0.06f, 0.05f), 7, ERT("赤"));
	SpawnAt<AERInspect>(FVector(400.f, -150.f, 175.f), 180.f)->BuildPainting(FLinearColor(0.05f, 0.15f, 0.75f), 2, ERT("青"));
	SpawnAt<AERInspect>(FVector(-400.f, 180.f, 160.f), 0.f)->BuildPainting(FLinearColor(0.06f, 0.5f, 0.1f), 9, ERT("緑"));
	SpawnAt<AERInspect>(FVector(-220.f, 400.f, 160.f), -90.f)->BuildPainting(FLinearColor(0.85f, 0.7f, 0.05f), 5, ERT("黄"));

	// ⑥ 扉とキーパッド
	AERDoor* Door = SpawnAt<AERDoor>(FVector(0.f, 410.f, 0.f), 0.f);
	Door->Build();
	SpawnAt<AERKeypad>(FVector(110.f, 400.f, 130.f), -90.f)->Build(TEXT("7295"), Door);

	// ⑦ 出口
	SpawnAt<AERExit>(FVector(0.f, 640.f, 110.f), 0.f);
}

void AERGameMode::ShowMessage(const FText& Msg, float Duration)
{
	Message = Msg;
	MessageEndTime = GetWorld()->GetTimeSeconds() + Duration;
}

void AERGameMode::ShowNote(const FText& Title, const FText& Body)
{
	NoteTitle = Title;
	NoteBody = Body;
	bNoteOpen = true;
}

FText AERGameMode::GetMessage() const
{
	return GetWorld()->GetTimeSeconds() < MessageEndTime ? Message : FText::GetEmpty();
}

FText AERGameMode::GetTimeText() const
{
	const double T = bEscaped ? ClearTime : GetWorld()->GetTimeSeconds();
	const int32 Sec = FMath::FloorToInt32(T);
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Sec / 60, Sec % 60));
}

void AERGameMode::OnEscaped()
{
	if (bEscaped)
	{
		return;
	}
	bEscaped = true;
	bNoteOpen = false;
	ClearTime = GetWorld()->GetTimeSeconds();
	Objective = ERT("脱出成功！");
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AERCharacter* C = Cast<AERCharacter>(PC->GetPawn()))
		{
			C->LockControls();
		}
	}
}
