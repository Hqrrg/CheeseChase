// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM(BlueprintType)
enum class ETileAttachLocation : uint8
{
	Forward = 0,
	Left,
	Right
};

UENUM(BlueprintType)
enum EMeshAlignment : uint8
{
	None = 0,
	Front,
	Rear,
	Left,
	Right
};

UENUM(BlueprintType)
enum class ETileLane : uint8
{
	Left = 0,
	Middle,
	Right
};

UCLASS()
class CHEESECHASE_API ATile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FloorMesh = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attachment", meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* NextAttachArrow = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TileBox = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lanes", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* LeftLaneSpline = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lanes", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* MiddleLaneSpline = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lanes", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* RightLaneSpline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FrontWallMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RearWallMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* LeftWallMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RightWallMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* FloorMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* FrontWallMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* RearWallMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* LeftWallMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Appearance", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* RightWallMeshAsset = nullptr;

public:
	// Sets default values for this actor's properties
	ATile();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FTransform GetNextAttachTransform() const;

	UFUNCTION(BlueprintPure)
	class USplineComponent* GetLaneSpline(ETileLane TileLane);

	FORCEINLINE bool IsCorner() const { return E_NextAttachLocation != ETileAttachLocation::Forward; }

private:
	UFUNCTION()
	void TileBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void TileBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	void UpdateMeshComponent(UStaticMeshComponent* MeshComponent, UStaticMesh* MeshAsset, EMeshAlignment Alignment = EMeshAlignment::None);
	void UpdateTileBox();
	void UpdateNextAttachArrow();
	void UpdateLanes();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Attachment", DisplayName = "Next Attach Location", meta = (AllowPrivateAccess = "true"))
	ETileAttachLocation E_NextAttachLocation = ETileAttachLocation::Forward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Lanes", meta = (AllowPrivateAccess = "true", ClampMin = "0.25", UIMin = "0.25", ClampMax = "1", UIMax = "1"))
	float LaneSpacingMultiplier = 1.0f;

private:
	UPROPERTY()
	class ACheeseChaseGameMode* GameMode = nullptr;
};
