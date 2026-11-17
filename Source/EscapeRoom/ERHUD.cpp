#include "ERHUD.h"

#include "ERCharacter.h"
#include "ERCommon.h"
#include "ERGameMode.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

AERHUD::AERHUD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AERHUD::BeginPlay()
{
	Super::BeginPlay();
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	TWeakObjectPtr<AERHUD> Self(this);
	auto GameMode = [Self]() -> AERGameMode*
	{
		return Self.IsValid() && Self->GetWorld() ? Self->GetWorld()->GetAuthGameMode<AERGameMode>() : nullptr;
	};
	auto Player = [Self]() -> AERCharacter*
	{
		return Self.IsValid() && Self->GetOwningPlayerController() ? Cast<AERCharacter>(Self->GetOwningPlayerController()->GetPawn()) : nullptr;
	};
	auto Font = [](int32 Size, const char* Style = "Regular") { return FCoreStyle::GetDefaultFontStyle(Style, Size); };
	auto ShowIf = [](TFunction<bool()> Pred)
	{
		return TAttribute<EVisibility>::CreateLambda([Pred]() { return Pred() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; });
	};

	const FSlateColor White(FLinearColor::White);
	const FSlateColor Ink(FLinearColor(0.08f, 0.06f, 0.04f));

	Root = SNew(SOverlay)
		.Visibility(EVisibility::HitTestInvisible)

		// 照準
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("+")))
			.Font(Font(16))
			.ColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1, 0.7f)))
			.Visibility(ShowIf([GameMode]() { AERGameMode* G = GameMode(); return G && !G->IsNoteOpen() && !G->IsEscaped(); }))
		]

		// 調べられる物の説明
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(0, 90, 0, 0))
		[
			SNew(STextBlock)
			.Font(Font(18, "Bold"))
			.ColorAndOpacity(White)
			.ShadowOffset(FVector2D(1, 1))
			.Text_Lambda([Player, GameMode]()
			{
				AERGameMode* G = GameMode();
				AERCharacter* P = Player();
				return (P && G && !G->IsNoteOpen()) ? P->GetFocusPrompt() : FText::GetEmpty();
			})
		]

		// 左上: 目的・所持品・時間・操作
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(14, 10))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(15, "Bold")).ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.4f)))
					.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? FText::Format(ERT("目的: {0}"), G->GetObjective()) : FText(); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
				[
					SNew(STextBlock).Font(Font(13)).ColorAndOpacity(White)
					.Text_Lambda([Player]() { AERCharacter* P = Player(); return P ? FText::Format(ERT("所持品: {0}"), P->GetInventoryText()) : FText(); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
				[
					SNew(STextBlock).Font(Font(13)).ColorAndOpacity(White)
					.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? FText::Format(ERT("経過時間: {0}"), G->GetTimeText()) : FText(); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
				[
					SNew(STextBlock).Font(Font(10)).ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f)))
					.Text(ERT("WASD: 移動　マウス: 視点　E / 左クリック: 調べる"))
				]
			]
		]

		// 上部中央: メッセージ
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(FMargin(0, 0, 0, 80))
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(20, 10))
			.Visibility(ShowIf([GameMode]() { AERGameMode* G = GameMode(); return G && !G->GetMessage().IsEmpty() && !G->IsEscaped(); }))
			[
				SNew(STextBlock).Font(Font(17)).ColorAndOpacity(White)
				.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? G->GetMessage() : FText(); })
			]
		]

		// メモを読む画面
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(620.f)
			.Visibility(ShowIf([GameMode]() { AERGameMode* G = GameMode(); return G && G->IsNoteOpen(); }))
			[
				SNew(SBorder)
				.BorderImage(&NoteBrush)
				.Padding(FMargin(36, 28))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
					[
						SNew(STextBlock).Font(Font(22, "Bold")).ColorAndOpacity(Ink)
						.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? G->GetNoteTitle() : FText(); })
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Font(Font(17)).ColorAndOpacity(Ink).AutoWrapText(true)
						.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? G->GetNoteBody() : FText(); })
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 22, 0, 0).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Font(Font(12)).ColorAndOpacity(FSlateColor(FLinearColor(0.35f, 0.3f, 0.25f)))
						.Text(ERT("[E] 閉じる"))
					]
				]
			]
		]

		// クリア画面
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(&ClearBrush)
			.HAlign(HAlign_Center).VAlign(VAlign_Center)
			.Visibility(ShowIf([GameMode]() { AERGameMode* G = GameMode(); return G && G->IsEscaped(); }))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock).Font(Font(48, "Bold")).ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 1.f, 0.5f)))
					.Text(ERT("脱出成功！"))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 18, 0, 0)
				[
					SNew(STextBlock).Font(Font(24)).ColorAndOpacity(White)
					.Text_Lambda([GameMode]() { AERGameMode* G = GameMode(); return G ? FText::Format(ERT("クリアタイム  {0}"), G->GetTimeText()) : FText(); })
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 30, 0, 0)
				[
					SNew(STextBlock).Font(Font(15)).ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
					.Text(ERT("[R] もう一度遊ぶ　　[Esc] 終了"))
				]
			]
		];

	GEngine->GameViewport->AddViewportWidgetContent(Root.ToSharedRef(), 10);
}

void AERHUD::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Root.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Root.ToSharedRef());
	}
	Root.Reset();
	Super::EndPlay(Reason);
}
