#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Brushes/SlateColorBrush.h"
#include "ERHUD.generated.h"

class SWidget;

/** UMG アセットを使わず Slate を直接組んだ軽量 HUD（日本語フォントはエンジン既定のフォールバックを使用） */
UCLASS()
class AERHUD : public AHUD
{
	GENERATED_BODY()

public:
	AERHUD();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	TSharedPtr<SWidget> Root;
	FSlateColorBrush PanelBrush = FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	FSlateColorBrush NoteBrush = FSlateColorBrush(FLinearColor(0.93f, 0.89f, 0.78f, 0.97f));
	FSlateColorBrush ClearBrush = FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.8f));
};
