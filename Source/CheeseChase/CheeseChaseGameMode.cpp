// Copyright Epic Games, Inc. All Rights Reserved.

#include "CheeseChaseGameMode.h"

#include "Tile.h"
#include "UObject/ConstructorHelpers.h"

ACheeseChaseGameMode::ACheeseChaseGameMode()
{
}

void ACheeseChaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	NextTileTransform = FTransform::Identity;
	SpawnTiles(10);
}

// I FUCKING LOVE RECURSION!!!!!
void ACheeseChaseGameMode::SpawnTiles(int32 Num)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TSubclassOf<ATile> TileClass = SpawningTileClass;
	
	if (!IsFirstTile)
	{
		TArray<TSubclassOf<ATile>> SortedTileClasses;

		for (const TPair<TSubclassOf<ATile>, ETileRarity>& Pair : TilePrefabs)
		{
			for (uint32 Index = 0; Index < static_cast<uint8>(Pair.Value); Index++)
			{
				SortedTileClasses.Add(Pair.Key);
			}
		}
		if (!SortedTileClasses.IsEmpty()) TileClass = SortedTileClasses[FMath::RandRange(0, SortedTileClasses.Num() - 1)];
	}

	if (!TileClass) return;

	FActorSpawnParameters SpawnParams;
	ATile* NextTile = World->SpawnActorDeferred<ATile>(TileClass->GetAuthoritativeClass(), NextTileTransform);

	if (NextTile)
	{
		if (NextTile->IsCorner())
		{
			if (CornerBuffer > 0)
			{
				NextTile->Destroy();
				SpawnTiles(Num);
				return;
			}
			CornerBuffer = MaxCornerBuffer;
		}
		
		NextTile->FinishSpawning(NextTileTransform);
		NextTileTransform = NextTile->GetNextAttachTransform();
		CornerBuffer--;
	}

	IsFirstTile = false;
	if (Num > 1) SpawnTiles(Num - 1);
}
