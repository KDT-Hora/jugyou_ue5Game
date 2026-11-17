#include "ERCharacter.h"

#include "ERCommon.h"
#include "ERGameMode.h"
#include "ERProps.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	constexpr float InteractDistance = 220.f;
}

AERCharacter::AERCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(90.f);

	// 重いポストエフェクトは念のためカメラ側でも無効化
	FPostProcessSettings& PP = Camera->PostProcessSettings;
	PP.bOverride_MotionBlurAmount = true;     PP.MotionBlurAmount = 0.f;
	PP.bOverride_BloomIntensity = true;       PP.BloomIntensity = 0.f;
	PP.bOverride_AmbientOcclusionIntensity = true; PP.AmbientOcclusionIntensity = 0.f;
	PP.bOverride_LensFlareIntensity = true;   PP.LensFlareIntensity = 0.f;
	PP.bOverride_VignetteIntensity = true;    PP.VignetteIntensity = 0.3f;

	// 一人称なのでメッシュは不要
	GetMesh()->SetComponentTickEnabled(false);
	GetMesh()->PrimaryComponentTick.bCanEverTick = false;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = 320.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->GravityScale = 1.5f;
	Move->bOrientRotationToMovement = false;
}

void AERCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AERCharacter::CreateInput()
{
	if (Mapping)
	{
		return;
	}
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;
	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;
	RestartAction = NewObject<UInputAction>(this, TEXT("IA_Restart"));
	RestartAction->ValueType = EInputActionValueType::Boolean;
	QuitAction = NewObject<UInputAction>(this, TEXT("IA_Quit"));
	QuitAction->ValueType = EInputActionValueType::Boolean;

	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));

	auto Map = [this](UInputAction* Action, const FKey& Key, bool bSwizzle, bool bNegate)
	{
		FEnhancedActionKeyMapping& M = Mapping->MapKey(Action, Key);
		if (bSwizzle)
		{
			M.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this)); // X -> Y
		}
		if (bNegate)
		{
			M.Modifiers.Add(NewObject<UInputModifierNegate>(this));
		}
	};

	Map(MoveAction, EKeys::W, true, false);
	Map(MoveAction, EKeys::S, true, true);
	Map(MoveAction, EKeys::D, false, false);
	Map(MoveAction, EKeys::A, false, true);
	Map(MoveAction, EKeys::Up, true, false);
	Map(MoveAction, EKeys::Down, true, true);
	Map(MoveAction, EKeys::Right, false, false);
	Map(MoveAction, EKeys::Left, false, true);
	Map(LookAction, EKeys::Mouse2D, false, false);
	Map(InteractAction, EKeys::E, false, false);
	Map(InteractAction, EKeys::LeftMouseButton, false, false);
	Map(RestartAction, EKeys::R, false, false);
	Map(QuitAction, EKeys::Escape, false, false);
}

void AERCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	CreateInput();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AERCharacter::OnMove);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AERCharacter::OnLook);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AERCharacter::OnInteract);
		EIC->BindAction(RestartAction, ETriggerEvent::Started, this, &AERCharacter::OnRestart);
		EIC->BindAction(QuitAction, ETriggerEvent::Started, this, &AERCharacter::OnQuit);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Sub->ClearAllMappings();
				Sub->AddMappingContext(Mapping, 0);
			}
		}
	}
}

void AERCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 視線の先を1本のレイで調べるだけ（非常に軽い）
	Focused.Reset();
	FocusedComp.Reset();
	if (bLocked || !Controller)
	{
		return;
	}
	FVector Start;
	FRotator Rot;
	Controller->GetPlayerViewPoint(Start, Rot);
	const FVector End = Start + Rot.Vector() * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ERInteract), false, this);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AERInteractable* I = Cast<AERInteractable>(Hit.GetActor()))
		{
			Focused = I;
			FocusedComp = Hit.GetComponent();
		}
	}
}

FText AERCharacter::GetFocusPrompt() const
{
	if (Focused.IsValid())
	{
		return Focused->GetPrompt(this, FocusedComp.Get());
	}
	return FText::GetEmpty();
}

FText AERCharacter::GetInventoryText() const
{
	if (Items.Num() == 0)
	{
		return ERT("なし");
	}
	TArray<FString> Names;
	for (const TPair<FName, FText>& P : Items)
	{
		Names.Add(P.Value.ToString());
	}
	return FText::FromString(FString::Join(Names, TEXT(", ")));
}

void AERCharacter::LockControls()
{
	bLocked = true;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
}

void AERCharacter::OnMove(const FInputActionValue& Value)
{
	if (bLocked)
	{
		return;
	}
	const FVector2D V = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), V.Y);
	AddMovementInput(GetActorRightVector(), V.X);
}

void AERCharacter::OnLook(const FInputActionValue& Value)
{
	if (bLocked)
	{
		return;
	}
	const FVector2D V = Value.Get<FVector2D>();
	AddControllerYawInput(V.X);
	AddControllerPitchInput(-V.Y);
}

void AERCharacter::OnInteract()
{
	AERGameMode* G = GetWorld()->GetAuthGameMode<AERGameMode>();
	if (G && G->IsNoteOpen())
	{
		G->CloseNote();
		return;
	}
	if (bLocked)
	{
		return;
	}
	if (Focused.IsValid())
	{
		Focused->Interact(this, FocusedComp.Get());
	}
}

void AERCharacter::OnRestart()
{
	AERGameMode* G = GetWorld()->GetAuthGameMode<AERGameMode>();
	if (G && G->IsEscaped())
	{
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
	}
}

void AERCharacter::OnQuit()
{
	UKismetSystemLibrary::QuitGame(this, Cast<APlayerController>(GetController()), EQuitPreference::Quit, false);
}
