#include "ERCommon.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"
#include "UObject/Package.h"

namespace
{
	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	}

	template <class T>
	T* Finish(AActor* Owner, USceneComponent* Parent, T* Comp, const FVector& Loc, const FRotator& Rot, bool bForceMovable = false)
	{
		USceneComponent* AttachTo = Parent ? Parent : Owner->GetRootComponent();
		const EComponentMobility::Type Mob = (bForceMovable || !AttachTo) ? EComponentMobility::Movable : AttachTo->Mobility.GetValue();
		Comp->SetMobility(Mob);
		Comp->SetupAttachment(AttachTo);
		Comp->SetRelativeLocationAndRotation(Loc, Rot);
		Comp->RegisterComponent();
		Owner->AddInstanceComponent(Comp);
		return Comp;
	}

	UStaticMeshComponent* AddMesh(AActor* Owner, USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Loc,
		const FVector& Scale, const FLinearColor& Color, const FRotator& Rot, bool bCastShadow)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner);
		C->SetStaticMesh(Mesh);
		C->SetMaterial(0, ER::ColorMat(Color));
		C->SetRelativeScale3D(Scale);
		C->SetCastShadow(bCastShadow);
		C->SetGenerateOverlapEvents(false);
		C->bUseAsOccluder = true;
		return Finish(Owner, Parent, C, Loc, Rot);
	}
}

UStaticMesh* ER::CubeMesh()     { static TWeakObjectPtr<UStaticMesh> M; if (!M.IsValid()) M = LoadMesh(TEXT("/Engine/BasicShapes/Cube.Cube")); return M.Get(); }
UStaticMesh* ER::CylinderMesh() { static TWeakObjectPtr<UStaticMesh> M; if (!M.IsValid()) M = LoadMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")); return M.Get(); }
UStaticMesh* ER::SphereMesh()   { static TWeakObjectPtr<UStaticMesh> M; if (!M.IsValid()) M = LoadMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")); return M.Get(); }

UMaterialInterface* ER::ColorMat(const FLinearColor& Color)
{
	static TMap<uint32, TWeakObjectPtr<UMaterialInstanceDynamic>> Cache;
	const uint32 Key = Color.ToFColor(true).ToPackedARGB();
	if (TWeakObjectPtr<UMaterialInstanceDynamic>* Found = Cache.Find(Key))
	{
		if (Found->IsValid())
		{
			return Found->Get();
		}
	}
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Base)
	{
		return nullptr;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, GetTransientPackage());
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	Cache.Add(Key, MID);
	return MID;
}

UStaticMeshComponent* ER::AddBox(AActor* Owner, USceneComponent* Parent, const FVector& Loc, const FVector& Size,
	const FLinearColor& Color, const FRotator& Rot, bool bCastShadow)
{
	return AddMesh(Owner, Parent, CubeMesh(), Loc, Size / 100.f, Color, Rot, bCastShadow);
}

UStaticMeshComponent* ER::AddDiscX(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
	float Thickness, const FLinearColor& Color)
{
	// 円柱は Z 軸向きなので Pitch 90 で X 軸向きにする
	return AddMesh(Owner, Parent, CylinderMesh(), Loc, FVector(Diameter, Diameter, Thickness) / 100.f, Color,
		FRotator(90.f, 0.f, 0.f), true);
}

UStaticMeshComponent* ER::AddCylinderZ(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
	float Height, const FLinearColor& Color)
{
	return AddMesh(Owner, Parent, CylinderMesh(), Loc, FVector(Diameter, Diameter, Height) / 100.f, Color,
		FRotator::ZeroRotator, true);
}

UStaticMeshComponent* ER::AddSphere(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
	const FLinearColor& Color)
{
	return AddMesh(Owner, Parent, SphereMesh(), Loc, FVector(Diameter) / 100.f, Color, FRotator::ZeroRotator, false);
}

UTextRenderComponent* ER::AddText(AActor* Owner, USceneComponent* Parent, const FVector& Loc, const FString& Text,
	float WorldSize, const FColor& Color, const FRotator& Rot)
{
	UTextRenderComponent* T = NewObject<UTextRenderComponent>(Owner);
	T->SetText(FText::FromString(Text));
	T->SetWorldSize(WorldSize);
	T->SetTextRenderColor(Color);
	T->SetHorizontalAlignment(EHTA_Center);
	T->SetVerticalAlignment(EVRTA_TextCenter);
	T->SetCastShadow(false);
	T->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return Finish(Owner, Parent, T, Loc, Rot);
}

UPointLightComponent* ER::AddPointLight(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Lumens,
	float Radius, const FLinearColor& Color, bool bCastShadows)
{
	UPointLightComponent* L = NewObject<UPointLightComponent>(Owner);
	L->SetIntensityUnits(ELightUnits::Lumens);
	L->SetIntensity(Lumens);
	L->SetAttenuationRadius(Radius);
	L->SetLightColor(Color);
	L->SetCastShadows(bCastShadows);
	L->SetSourceRadius(0.f);
	L->SetSoftSourceRadius(0.f);
	L->bUseInverseSquaredFalloff = true;
	return Finish(Owner, Parent, L, Loc, FRotator::ZeroRotator, true);
}

USceneComponent* ER::AddPivot(AActor* Owner, USceneComponent* Parent, const FVector& Loc)
{
	USceneComponent* S = NewObject<USceneComponent>(Owner);
	return Finish(Owner, Parent, S, Loc, FRotator::ZeroRotator);
}
