#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ERProps.generated.h"

class AERCharacter;
class AERGameMode;
class AERDoor;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UBoxComponent;

/** 調べられる物の基底クラス。普段は Tick しない（扉が動く間だけ Tick） */
UCLASS(Abstract)
class AERInteractable : public AActor
{
	GENERATED_BODY()

public:
	AERInteractable();

	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const { return FText::GetEmpty(); }
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) {}

	/** 隠す/出す（金庫の中身など） */
	void SetAvailable(bool bAvailable);

	virtual void Tick(float DeltaSeconds) override;

protected:
	AERGameMode* GM() const;
	void AnimateHinge(USceneComponent* InHinge, float TargetYaw);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> Hinge;
	float HingeTarget = 0.f;
};

/** 読めるメモ */
UCLASS()
class AERNote : public AERInteractable
{
	GENERATED_BODY()
public:
	void Init(const FText& InTitle, const FText& InBody, const FText& InObjectiveAfterRead);
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	FText Title, Body, ObjectiveAfterRead;
};

/** 拾えるアイテム（鍵） */
UCLASS()
class AERPickup : public AERInteractable
{
	GENERATED_BODY()
public:
	void Init(FName InItemId, const FText& InName);
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	FName ItemId;
	FText ItemName;
};

/** 調べると一言メッセージが出るだけの物（時計・絵など） */
UCLASS()
class AERInspect : public AERInteractable
{
	GENERATED_BODY()
public:
	void SetInfo(const FText& InPrompt, const FText& InMessage) { Prompt = InPrompt; Message = InMessage; }
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override { return Prompt; }
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;

	/** 止まった時計を作る */
	void BuildClock(int32 Hour, int32 Minute);
	/** 数字の描かれた色付きの絵を作る */
	void BuildPainting(const FLinearColor& Color, int32 Digit, const FText& ColorName);
private:
	FText Prompt, Message;
};

/** 3桁ダイヤル式の金庫 */
UCLASS()
class AERSafe : public AERInteractable
{
	GENERATED_BODY()
public:
	void Build(const TArray<int32>& InCode, AERInteractable* InReward);
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	int32 DialIndexOf(const UPrimitiveComponent* Comp) const;

	UPROPERTY() TObjectPtr<USceneComponent> DoorHinge;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Dials;
	UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> DialTexts;
	UPROPERTY() TObjectPtr<AERInteractable> Reward;
	TArray<int32> Code;
	TArray<int32> Current;
	bool bOpen = false;
};

/** 鍵で開く戸棚 */
UCLASS()
class AERCabinet : public AERInteractable
{
	GENERATED_BODY()
public:
	void Build(FName InKeyId, AERInteractable* InReward);
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	UPROPERTY() TObjectPtr<USceneComponent> DoorHinge;
	UPROPERTY() TObjectPtr<AERInteractable> Reward;
	FName KeyId;
	bool bOpen = false;
};

/** 出口の扉 */
UCLASS()
class AERDoor : public AERInteractable
{
	GENERATED_BODY()
public:
	void Build();
	void Open();
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	UPROPERTY() TObjectPtr<USceneComponent> DoorHinge;
	bool bOpen = false;
};

/** 扉の横の暗証キーパッド */
UCLASS()
class AERKeypad : public AERInteractable
{
	GENERATED_BODY()
public:
	void Build(const FString& InCode, AERDoor* InDoor);
	virtual FText GetPrompt(const AERCharacter* Player, const UPrimitiveComponent* Comp) const override;
	virtual void Interact(AERCharacter* Player, UPrimitiveComponent* Comp) override;
private:
	void Refresh();

	UPROPERTY() TMap<TObjectPtr<UPrimitiveComponent>, FString> Buttons;
	UPROPERTY() TObjectPtr<UTextRenderComponent> Display;
	UPROPERTY() TObjectPtr<AERDoor> Door;
	FString Code;
	FString Entry;
	bool bSolved = false;
};

/** 出口のトリガー */
UCLASS()
class AERExit : public AActor
{
	GENERATED_BODY()
public:
	AERExit();
private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY() TObjectPtr<UBoxComponent> Box;
};
