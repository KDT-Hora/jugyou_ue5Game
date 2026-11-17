#include "ERProps.h"

#include "ERCommon.h"
#include "ERCharacter.h"
#include "ERGameMode.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Math/RotationMatrix.h"

namespace
{
	const FLinearColor Paper(0.85f, 0.80f, 0.65f);
	const FLinearColor Gold(0.9f, 0.65f, 0.15f);
	const FLinearColor DarkMetal(0.12f, 0.14f, 0.15f);
	const FLinearColor Wood(0.30f, 0.17f, 0.08f);
	const FLinearColor WoodLight(0.42f, 0.25f, 0.12f);
}

// ---------------------------------------------------------------- Interactable

AERInteractable::AERInteractable()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // 必要な時だけ Tick
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

AERGameMode* AERInteractable::GM() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AERGameMode>() : nullptr;
}

void AERInteractable::SetAvailable(bool bAvailable)
{
	SetActorHiddenInGame(!bAvailable);
	SetActorEnableCollision(bAvailable);
}

void AERInteractable::AnimateHinge(USceneComponent* InHinge, float TargetYaw)
{
	Hinge = InHinge;
	HingeTarget = TargetYaw;
	SetActorTickEnabled(true);
}

void AERInteractable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Hinge)
	{
		SetActorTickEnabled(false);
		return;
	}
	FRotator R = Hinge->GetRelativeRotation();
	R.Yaw = FMath::FInterpTo(R.Yaw, HingeTarget, DeltaSeconds, 4.f);
	if (FMath::Abs(R.Yaw - HingeTarget) < 0.2f)
	{
		R.Yaw = HingeTarget;
		SetActorTickEnabled(false); // 動き終わったら Tick 停止
	}
	Hinge->SetRelativeRotation(R);
}

// ---------------------------------------------------------------- Note

void AERNote::Init(const FText& InTitle, const FText& InBody, const FText& InObjectiveAfterRead)
{
	Title = InTitle;
	Body = InBody;
	ObjectiveAfterRead = InObjectiveAfterRead;
	ER::AddBox(this, nullptr, FVector(0, 0, 0.4f), FVector(22, 30, 0.8f), Paper, FRotator::ZeroRotator, false);
	// 文字が書いてあるように見せる線
	for (int32 i = 0; i < 4; ++i)
	{
		ER::AddBox(this, nullptr, FVector(-6.f + i * 4.f, 0, 0.85f), FVector(0.6f, 22, 0.1f),
			FLinearColor(0.1f, 0.1f, 0.12f), FRotator::ZeroRotator, false);
	}
}

FText AERNote::GetPrompt(const AERCharacter*, const UPrimitiveComponent*) const
{
	return FText::Format(ERT("E: {0} を読む"), Title);
}

void AERNote::Interact(AERCharacter*, UPrimitiveComponent*)
{
	if (AERGameMode* G = GM())
	{
		G->ShowNote(Title, Body);
		if (!ObjectiveAfterRead.IsEmpty())
		{
			G->SetObjective(ObjectiveAfterRead);
			ObjectiveAfterRead = FText::GetEmpty();
		}
	}
}

// ---------------------------------------------------------------- Pickup

void AERPickup::Init(FName InItemId, const FText& InName)
{
	ItemId = InItemId;
	ItemName = InName;
	// 金色の鍵
	ER::AddCylinderZ(this, nullptr, FVector(0, 7, 1), 7, 1.5f, Gold);
	ER::AddBox(this, nullptr, FVector(0, -2, 1), FVector(1.8f, 13, 1.5f), Gold);
	ER::AddBox(this, nullptr, FVector(-1.8f, -7, 1), FVector(2.5f, 1.5f, 1.5f), Gold);
	ER::AddBox(this, nullptr, FVector(-1.8f, -4, 1), FVector(2.5f, 1.5f, 1.5f), Gold);
}

FText AERPickup::GetPrompt(const AERCharacter*, const UPrimitiveComponent*) const
{
	return FText::Format(ERT("E: {0} を拾う"), ItemName);
}

void AERPickup::Interact(AERCharacter* Player, UPrimitiveComponent*)
{
	if (!Player)
	{
		return;
	}
	Player->AddItem(ItemId, ItemName);
	if (AERGameMode* G = GM())
	{
		G->ShowMessage(FText::Format(ERT("{0} を手に入れた。"), ItemName));
		G->SetObjective(ERT("鍵の使い道を探そう"));
	}
	Destroy();
}

// ---------------------------------------------------------------- Inspect (clock / painting)

void AERInspect::Interact(AERCharacter*, UPrimitiveComponent*)
{
	if (AERGameMode* G = GM())
	{
		G->ShowMessage(Message, 4.f);
	}
}

void AERInspect::BuildClock(int32 Hour, int32 Minute)
{
	ER::AddDiscX(this, nullptr, FVector(1.5f, 0, 0), 78, 3, FLinearColor(0.1f, 0.08f, 0.06f));
	ER::AddDiscX(this, nullptr, FVector(3.5f, 0, 0), 70, 3, FLinearColor(0.9f, 0.88f, 0.82f));

	const FColor Ink(20, 20, 20);
	ER::AddText(this, nullptr, FVector(5.5f, 0, 27), TEXT("12"), 9, Ink);
	ER::AddText(this, nullptr, FVector(5.5f, -27, 0), TEXT("3"), 9, Ink);
	ER::AddText(this, nullptr, FVector(5.5f, 0, -27), TEXT("6"), 9, Ink);
	ER::AddText(this, nullptr, FVector(5.5f, 27, 0), TEXT("9"), 9, Ink);

	// 針: 12時 = +Z、3時 = -Y（見る人の右）
	auto AddHand = [this](float AngleDeg, float Length, float Width, float X)
	{
		const float Rad = FMath::DegreesToRadians(AngleDeg);
		const FVector Dir(0.f, -FMath::Sin(Rad), FMath::Cos(Rad));
		const FRotator Rot = FRotationMatrix::MakeFromXZ(FVector::ForwardVector, Dir).Rotator();
		ER::AddBox(this, nullptr, FVector(X, 0, 0) + Dir * (Length * 0.5f), FVector(1.f, Width, Length),
			FLinearColor(0.05f, 0.05f, 0.05f), Rot, false);
	};
	AddHand((Hour % 12 + Minute / 60.f) * 30.f, 16.f, 3.f, 5.5f);
	AddHand(Minute * 6.f, 27.f, 2.f, 6.5f);
	ER::AddSphere(this, nullptr, FVector(6.5f, 0, 0), 4, FLinearColor(0.6f, 0.1f, 0.1f));

	SetInfo(ERT("E: 時計を調べる"), ERT("秒針が動いていない。この時計は止まっている…。"));
}

void AERInspect::BuildPainting(const FLinearColor& Color, int32 Digit, const FText& ColorName)
{
	ER::AddBox(this, nullptr, FVector(1.5f, 0, 0), FVector(3, 56, 66), Color);
	ER::AddBox(this, nullptr, FVector(3.2f, 0, 0), FVector(1, 46, 56), FLinearColor(0.92f, 0.9f, 0.84f), FRotator::ZeroRotator, false);
	ER::AddText(this, nullptr, FVector(4.f, 0, 0), FString::FromInt(Digit), 36, (Color * 0.45f).ToFColor(true));

	SetInfo(ERT("E: 絵を調べる"),
		FText::FromString(FString::Printf(TEXT("%s色の額縁の絵だ。大きく「%d」と描かれている。"), *ColorName.ToString(), Digit)));
}

// ---------------------------------------------------------------- Safe

void AERSafe::Build(const TArray<int32>& InCode, AERInteractable* InReward)
{
	Code = InCode;
	Current.Init(0, Code.Num());
	Reward = InReward;
	if (Reward)
	{
		Reward->SetAvailable(false);
	}

	// 中が空洞の箱（ローカル +X が正面）
	ER::AddBox(this, nullptr, FVector(2.5f, 0, 35), FVector(5, 80, 70), DarkMetal);
	ER::AddBox(this, nullptr, FVector(30, 37.5f, 35), FVector(60, 5, 70), DarkMetal);
	ER::AddBox(this, nullptr, FVector(30, -37.5f, 35), FVector(60, 5, 70), DarkMetal);
	ER::AddBox(this, nullptr, FVector(30, 0, 2.5f), FVector(60, 70, 5), DarkMetal);
	ER::AddBox(this, nullptr, FVector(30, 0, 67.5f), FVector(60, 70, 5), DarkMetal);

	DoorHinge = ER::AddPivot(this, nullptr, FVector(62, 40, 0));
	ER::AddBox(this, DoorHinge, FVector(0, -40, 35), FVector(4, 80, 70), FLinearColor(0.2f, 0.23f, 0.25f));
	ER::AddBox(this, DoorHinge, FVector(3, -70, 30), FVector(3, 4, 16), FLinearColor(0.6f, 0.6f, 0.6f));
	ER::AddText(this, DoorHinge, FVector(2.5f, -40, 60), TEXT("SAFE"), 6, FColor(200, 200, 200));

	for (int32 i = 0; i < Code.Num(); ++i)
	{
		const float Y = -22.f - 18.f * i; // 見る人の左から右へ
		Dials.Add(ER::AddDiscX(this, DoorHinge, FVector(3, Y, 42), 14, 3, FLinearColor(0.85f, 0.85f, 0.8f)));
		DialTexts.Add(ER::AddText(this, DoorHinge, FVector(5, Y, 42), TEXT("0"), 10, FColor(15, 15, 15)));
	}
}

int32 AERSafe::DialIndexOf(const UPrimitiveComponent* Comp) const
{
	for (int32 i = 0; i < Dials.Num(); ++i)
	{
		if (Dials[i] == Comp)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

FText AERSafe::GetPrompt(const AERCharacter*, const UPrimitiveComponent* Comp) const
{
	if (bOpen)
	{
		return FText::GetEmpty();
	}
	const int32 Idx = DialIndexOf(Comp);
	if (Idx != INDEX_NONE)
	{
		return FText::FromString(FString::Printf(TEXT("E: %d番目のダイヤルを回す"), Idx + 1));
	}
	return ERT("金庫 (3桁のダイヤル錠)");
}

void AERSafe::Interact(AERCharacter*, UPrimitiveComponent* Comp)
{
	if (bOpen)
	{
		return;
	}
	const int32 Idx = DialIndexOf(Comp);
	if (Idx == INDEX_NONE)
	{
		if (AERGameMode* G = GM())
		{
			G->ShowMessage(ERT("3つのダイヤルで開く金庫だ。番号さえ分かれば…。"));
		}
		return;
	}

	Current[Idx] = (Current[Idx] + 1) % 10;
	DialTexts[Idx]->SetText(FText::AsNumber(Current[Idx]));

	if (Current == Code)
	{
		bOpen = true;
		AnimateHinge(DoorHinge, 100.f);
		if (Reward)
		{
			Reward->SetAvailable(true);
		}
		if (AERGameMode* G = GM())
		{
			G->ShowMessage(ERT("カチッ… 金庫が開いた！ 中に何か入っている。"));
			G->SetObjective(ERT("金庫の中を調べよう"));
		}
	}
}

// ---------------------------------------------------------------- Cabinet

void AERCabinet::Build(FName InKeyId, AERInteractable* InReward)
{
	KeyId = InKeyId;
	Reward = InReward;
	if (Reward)
	{
		Reward->SetAvailable(false);
	}

	ER::AddBox(this, nullptr, FVector(2, 0, 90), FVector(4, 90, 180), Wood);
	ER::AddBox(this, nullptr, FVector(25, 43, 90), FVector(50, 4, 180), Wood);
	ER::AddBox(this, nullptr, FVector(25, -43, 90), FVector(50, 4, 180), Wood);
	ER::AddBox(this, nullptr, FVector(25, 0, 178), FVector(50, 90, 4), Wood);
	ER::AddBox(this, nullptr, FVector(25, 0, 2), FVector(50, 90, 4), Wood);
	ER::AddBox(this, nullptr, FVector(25, 0, 86), FVector(46, 82, 3), Wood);

	DoorHinge = ER::AddPivot(this, nullptr, FVector(52, 45, 0));
	ER::AddBox(this, DoorHinge, FVector(0, -45, 90), FVector(4, 90, 180), WoodLight);
	ER::AddBox(this, DoorHinge, FVector(2.2f, -80, 95), FVector(1, 6, 12), Gold);
	ER::AddBox(this, DoorHinge, FVector(2.8f, -80, 94), FVector(1, 1.5f, 5), FLinearColor::Black, FRotator::ZeroRotator, false);
}

FText AERCabinet::GetPrompt(const AERCharacter* Player, const UPrimitiveComponent*) const
{
	if (bOpen)
	{
		return FText::GetEmpty();
	}
	if (Player && Player->HasItem(KeyId))
	{
		return ERT("E: 鍵を使って戸棚を開ける");
	}
	return ERT("戸棚 (鍵がかかっている)");
}

void AERCabinet::Interact(AERCharacter* Player, UPrimitiveComponent*)
{
	if (bOpen || !Player)
	{
		return;
	}
	AERGameMode* G = GM();
	if (!Player->HasItem(KeyId))
	{
		if (G) G->ShowMessage(ERT("鍵がかかっている。鍵穴があるようだ。"));
		return;
	}
	Player->RemoveItem(KeyId);
	bOpen = true;
	AnimateHinge(DoorHinge, 100.f);
	if (Reward)
	{
		Reward->SetAvailable(true);
	}
	if (G)
	{
		G->ShowMessage(ERT("ガチャリ… 戸棚が開いた。"));
		G->SetObjective(ERT("戸棚の中を調べよう"));
	}
}

// ---------------------------------------------------------------- Door

void AERDoor::Build()
{
	DoorHinge = ER::AddPivot(this, nullptr, FVector(-60, 0, 0));
	ER::AddBox(this, DoorHinge, FVector(60, 0, 110), FVector(120, 8, 220), FLinearColor(0.22f, 0.12f, 0.06f));
	ER::AddBox(this, DoorHinge, FVector(60, -4.5f, 160), FVector(90, 1, 70), FLinearColor(0.28f, 0.16f, 0.08f), FRotator::ZeroRotator, false);
	ER::AddBox(this, DoorHinge, FVector(60, -4.5f, 60), FVector(90, 1, 80), FLinearColor(0.28f, 0.16f, 0.08f), FRotator::ZeroRotator, false);
	ER::AddSphere(this, DoorHinge, FVector(105, -7, 100), 7, Gold);
	ER::AddSphere(this, DoorHinge, FVector(105, 7, 100), 7, Gold);
}

void AERDoor::Open()
{
	if (!bOpen)
	{
		bOpen = true;
		AnimateHinge(DoorHinge, 90.f);
	}
}

FText AERDoor::GetPrompt(const AERCharacter*, const UPrimitiveComponent*) const
{
	return bOpen ? FText::GetEmpty() : ERT("扉 (ロックされている)");
}

void AERDoor::Interact(AERCharacter*, UPrimitiveComponent*)
{
	if (!bOpen)
	{
		if (AERGameMode* G = GM())
		{
			G->ShowMessage(ERT("ビクともしない。横のキーパッドで暗証番号を入力するようだ。"));
		}
	}
}

// ---------------------------------------------------------------- Keypad

void AERKeypad::Build(const FString& InCode, AERDoor* InDoor)
{
	Code = InCode;
	Door = InDoor;

	ER::AddBox(this, nullptr, FVector(1.5f, 0, 0), FVector(3, 32, 52), FLinearColor(0.08f, 0.08f, 0.09f));
	ER::AddBox(this, nullptr, FVector(3.2f, 0, 17), FVector(1, 26, 10), FLinearColor(0.01f, 0.02f, 0.01f), FRotator::ZeroRotator, false);
	Display = ER::AddText(this, nullptr, FVector(4.f, 0, 17), TEXT(""), 7, FColor(60, 255, 90));

	static const TCHAR* Labels[4][3] = {
		{ TEXT("1"), TEXT("2"), TEXT("3") },
		{ TEXT("4"), TEXT("5"), TEXT("6") },
		{ TEXT("7"), TEXT("8"), TEXT("9") },
		{ TEXT("C"), TEXT("0"), TEXT("OK") },
	};
	const float Cols[3] = { 9.f, 0.f, -9.f };
	const float Rows[4] = { 5.f, -4.f, -13.f, -22.f };
	for (int32 r = 0; r < 4; ++r)
	{
		for (int32 c = 0; c < 3; ++c)
		{
			const FString L = Labels[r][c];
			FLinearColor BtnColor(0.75f, 0.75f, 0.72f);
			if (L == TEXT("OK")) BtnColor = FLinearColor(0.2f, 0.6f, 0.25f);
			if (L == TEXT("C"))  BtnColor = FLinearColor(0.7f, 0.2f, 0.15f);
			UStaticMeshComponent* Btn = ER::AddBox(this, nullptr, FVector(4, Cols[c], Rows[r]), FVector(2, 7.5f, 7.5f), BtnColor, FRotator::ZeroRotator, false);
			ER::AddText(this, nullptr, FVector(5.2f, Cols[c], Rows[r]), L, L.Len() > 1 ? 3.5f : 5.f, FColor(10, 10, 10));
			Buttons.Add(Btn, L);
		}
	}
	Refresh();
}

void AERKeypad::Refresh()
{
	FString S = Entry;
	while (S.Len() < Code.Len())
	{
		S += TEXT("_");
	}
	Display->SetTextRenderColor(FColor(60, 255, 90));
	Display->SetText(FText::FromString(S));
}

FText AERKeypad::GetPrompt(const AERCharacter*, const UPrimitiveComponent* Comp) const
{
	if (bSolved)
	{
		return FText::GetEmpty();
	}
	if (const FString* L = Buttons.Find(const_cast<UPrimitiveComponent*>(Comp)))
	{
		return FText::FromString(FString::Printf(TEXT("E: [%s] を押す"), **L));
	}
	return ERT("キーパッド (4桁の暗証番号)");
}

void AERKeypad::Interact(AERCharacter*, UPrimitiveComponent* Comp)
{
	if (bSolved)
	{
		return;
	}
	const FString* L = Buttons.Find(Comp);
	if (!L)
	{
		return;
	}
	AERGameMode* G = GM();

	if (*L == TEXT("C"))
	{
		Entry.Reset();
		Refresh();
	}
	else if (*L == TEXT("OK"))
	{
		if (Entry == Code)
		{
			bSolved = true;
			Display->SetText(FText::FromString(TEXT("OPEN")));
			if (Door) Door->Open();
			if (G)
			{
				G->ShowMessage(ERT("ピピッ… ロックが解除された！"));
				G->SetObjective(ERT("開いた扉から外へ出よう！"));
			}
		}
		else
		{
			Entry.Reset();
			Display->SetTextRenderColor(FColor(255, 50, 40));
			Display->SetText(FText::FromString(TEXT("ERROR")));
			if (G) G->ShowMessage(ERT("ブーッ… 暗証番号が違うようだ。"));
		}
	}
	else if (Entry.Len() < Code.Len())
	{
		Entry += *L;
		Refresh();
	}
}

// ---------------------------------------------------------------- Exit

AERExit::AERExit()
{
	PrimaryActorTick.bCanEverTick = false;
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(70, 30, 110));
	Box->SetCollisionProfileName(TEXT("Trigger"));
	Box->SetGenerateOverlapEvents(true);
	RootComponent = Box;
	Box->OnComponentBeginOverlap.AddDynamic(this, &AERExit::OnOverlap);
}

void AERExit::OnOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (Cast<AERCharacter>(OtherActor))
	{
		if (AERGameMode* G = GetWorld()->GetAuthGameMode<AERGameMode>())
		{
			G->OnEscaped();
		}
	}
}
