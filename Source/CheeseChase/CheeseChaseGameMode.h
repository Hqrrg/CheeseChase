// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CheeseChaseGameMode.generated.h"

UENUM(BlueprintType)
enum class ETileRarity : uint8
{
	NONE = 0 UMETA(Hidden),
	RARE = 1,
	UNCOMMON = 2,
	COMMON = 4
};

UCLASS(minimalapi)
class ACheeseChaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACheeseChaseGameMode();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnTiles(int32 Num);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Tiles", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATile> SpawningTileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "*|Tiles", meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<class ATile>, ETileRarity> TilePrefabs;
	
	FTransform NextTileTransform;
	bool IsFirstTile = true;
	int32 MaxCornerBuffer = 3;
	int32 CornerBuffer = MaxCornerBuffer;
};



