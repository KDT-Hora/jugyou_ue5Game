#pragma once

#include "CoreMinimal.h"

class AActor;
class USceneComponent;
class UStaticMesh;
class UMaterialInterface;
class UStaticMeshComponent;
class UTextRenderComponent;
class UPointLightComponent;

// 日本語テキスト用の短縮マクロ
#define ERT(x) FText::FromString(TEXT(x))

/**
 * 実行時にエンジン標準の基本形状（/Engine/BasicShapes）だけで部屋を組み立てるためのヘルパー。
 * 外部アセットを一切使わないので、プロジェクトが軽量で済む。
 * 座標系の約束: 壁に付ける物は「ローカル +X が部屋側（プレイヤー側）」を向くように組む。
 */
namespace ER
{
	UStaticMesh* CubeMesh();
	UStaticMesh* CylinderMesh();
	UStaticMesh* SphereMesh();

	/** 色ごとに1つだけ作ってキャッシュするマテリアル（描画状態の切替を減らす） */
	UMaterialInterface* ColorMat(const FLinearColor& Color);

	/** 箱を追加。Loc = 中心、Size = 実寸(cm) */
	UStaticMeshComponent* AddBox(AActor* Owner, USceneComponent* Parent, const FVector& Loc, const FVector& Size,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator, bool bCastShadow = true);

	/** X 軸向きの円柱（壁に貼る円盤やダイヤル用）。Diameter と Thickness は cm */
	UStaticMeshComponent* AddDiscX(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
		float Thickness, const FLinearColor& Color);

	/** Z 軸向きの円柱 */
	UStaticMeshComponent* AddCylinderZ(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
		float Height, const FLinearColor& Color);

	UStaticMeshComponent* AddSphere(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Diameter,
		const FLinearColor& Color);

	/** +X 方向を向いたテキスト */
	UTextRenderComponent* AddText(AActor* Owner, USceneComponent* Parent, const FVector& Loc, const FString& Text,
		float WorldSize, const FColor& Color, const FRotator& Rot = FRotator::ZeroRotator);

	UPointLightComponent* AddPointLight(AActor* Owner, USceneComponent* Parent, const FVector& Loc, float Lumens,
		float Radius, const FLinearColor& Color, bool bCastShadows);

	/** 親コンポーネントを作る（ヒンジ等） */
	USceneComponent* AddPivot(AActor* Owner, USceneComponent* Parent, const FVector& Loc);
}
