#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ERCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AERInteractable;
struct FInputActionValue;

/** 一人称プレイヤー。入力アセットは実行時に C++ で生成する（.uasset 不要） */
UCLASS()
class AERCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AERCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** HUD 用: 今見ている物の説明 */
	FText GetFocusPrompt() const;

	bool HasItem(FName Id) const { return Items.Contains(Id); }
	void AddItem(FName Id, const FText& Name) { Items.Add(Id, Name); }
	void RemoveItem(FName Id) { Items.Remove(Id); }
	FText GetInventoryText() const;

	void LockControls();

private:
	void CreateInput();
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnInteract();
	void OnRestart();
	void OnQuit();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
	UPROPERTY() TObjectPtr<UInputAction> MoveAction;
	UPROPERTY() TObjectPtr<UInputAction> LookAction;
	UPROPERTY() TObjectPtr<UInputAction> InteractAction;
	UPROPERTY() TObjectPtr<UInputAction> RestartAction;
	UPROPERTY() TObjectPtr<UInputAction> QuitAction;

	TWeakObjectPtr<AERInteractable> Focused;
	TWeakObjectPtr<UPrimitiveComponent> FocusedComp;

	TMap<FName, FText> Items;
	bool bLocked = false;
};
