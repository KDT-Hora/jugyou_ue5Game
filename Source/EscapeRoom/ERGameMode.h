#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ERGameMode.generated.h"

/**
 * ゲーム全体の進行管理。開始時に部屋とギミックを全て C++ で生成する。
 * PlayerStart も不要（固定位置にスポーン）。
 */
UCLASS()
class AERGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AERGameMode();

	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void ShowMessage(const FText& Msg, float Duration = 3.5f);
	void ShowNote(const FText& Title, const FText& Body);
	void CloseNote() { bNoteOpen = false; }
	void SetObjective(const FText& InObjective) { Objective = InObjective; }
	void OnEscaped();

	bool IsNoteOpen() const { return bNoteOpen; }
	bool IsEscaped() const { return bEscaped; }
	FText GetMessage() const;
	FText GetObjective() const { return Objective; }
	FText GetNoteTitle() const { return NoteTitle; }
	FText GetNoteBody() const { return NoteBody; }
	FText GetTimeText() const;

private:
	void BuildLevel();

	template <class T>
	T* SpawnAt(const FVector& Loc, float Yaw);

	FText Message;
	double MessageEndTime = 0.0;
	FText Objective;
	FText NoteTitle;
	FText NoteBody;
	bool bNoteOpen = false;
	bool bEscaped = false;
	double ClearTime = 0.0;
};
