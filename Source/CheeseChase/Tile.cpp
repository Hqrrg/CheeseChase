// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

#include "CheeseChaseCharacter.h"
#include "CheeseChaseGameMode.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ATile::ATile()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	FloorMesh->SetupAttachment(Root);

	NextAttachArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("NextAttach"));
	NextAttachArrow->SetupAttachment(FloorMesh);
	NextAttachArrow->SetArrowColor(FLinearColor::Red);

	TileBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TileBox"));
	TileBox->ShapeColor = FColor::Yellow;
	TileBox->SetupAttachment(FloorMesh);

	LeftLaneSpline = CreateDefaultSubobject<USplineComponent>(TEXT("LeftLane"));
	LeftLaneSpline->SetupAttachment(FloorMesh);

	MiddleLaneSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MiddleLane"));
	MiddleLaneSpline->SetupAttachment(FloorMesh);

	RightLaneSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RightLane"));
	RightLaneSpline->SetupAttachment(FloorMesh);
	
	FrontWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWallMesh"));
	FrontWallMesh->SetupAttachment(FloorMesh);

	RearWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearWallMesh"));
	RearWallMesh->SetupAttachment(FloorMesh);

	LeftWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWallMesh"));
	LeftWallMesh->SetupAttachment(FloorMesh);

	RightWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWallMesh"));
	RightWallMesh->SetupAttachment(FloorMesh);
}

void ATile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateMeshComponent(FloorMesh, FloorMeshAsset);
	UpdateMeshComponent(FrontWallMesh, FrontWallMeshAsset, EMeshAlignment::Front);
	UpdateMeshComponent(RearWallMesh, RearWallMeshAsset, EMeshAlignment::Rear);
	UpdateMeshComponent(LeftWallMesh, LeftWallMeshAsset, EMeshAlignment::Left);
	UpdateMeshComponent(RightWallMesh, RightWallMeshAsset, EMeshAlignment::Right);

	UpdateTileBox();
	UpdateNextAttachArrow();
	UpdateLanes();
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	if (World)
	{
		GameMode = Cast<ACheeseChaseGameMode>(UGameplayStatics::GetGameMode(World));
	}
	
	TileBox->OnComponentBeginOverlap.AddDynamic(this, &ATile::TileBoxBeginOverlap);
	TileBox->OnComponentEndOverlap.AddDynamic(this, &ATile::TileBoxEndOverlap);
}

FTransform ATile::GetNextAttachTransform() const
{
	return NextAttachArrow->GetComponentTransform();
}

class USplineComponent* ATile::GetLaneSpline(ETileLane TileLane)
{
	USplineComponent* Spline = nullptr;
	
	switch (TileLane)
	{
	case ETileLane::Left:
		Spline = LeftLaneSpline;
		break;
	case ETileLane::Middle:
		Spline = MiddleLaneSpline;
		break;
	case ETileLane::Right:
		Spline = RightLaneSpline;
		break;
	default:
		break;
	}

	return Spline;
}

void ATile::TileBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACheeseChaseCharacter* Player = Cast<ACheeseChaseCharacter>(OtherActor))
	{
		Player->SetCurrentTile(this);
	}
}

void ATile::TileBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACheeseChaseCharacter* Player = Cast<ACheeseChaseCharacter>(OtherActor))
	{
		if (GameMode)
		{
			GameMode->SpawnTiles(1);
		}
	}
}

void ATile::UpdateMeshComponent(UStaticMeshComponent* MeshComponent, UStaticMesh* MeshAsset, EMeshAlignment Alignment)
{
	MeshComponent->SetStaticMesh(MeshAsset);

	if (!MeshAsset)
	{
		MeshComponent->SetRelativeLocation(FVector::ZeroVector);
		return;
	}

	USceneComponent* ParentMesh = MeshComponent->GetAttachParent();
	FVector MeshBounds = MeshAsset->GetBounds().BoxExtent;

	if (ParentMesh == RootComponent)
	{
		MeshComponent->SetRelativeLocation(FVector(MeshBounds.X, 0.0f, 0.0f));
	}
	
	if (!Alignment) return;
	
	FVector ParentBounds = ParentMesh->GetLocalBounds().BoxExtent;
	
	FVector OffsetDirection = FVector::ZeroVector;
	float OffsetAmount = ParentBounds.X + MeshBounds.X;

	switch (Alignment)
	{
	case EMeshAlignment::Front:
		OffsetDirection.X = 1.0f;
		break;
		
	case EMeshAlignment::Rear:
		OffsetDirection.X = -1.0f;
		break;
		
	case EMeshAlignment::Left:
		OffsetDirection.Y = -1.0f;
		break;
		
	case EMeshAlignment::Right:
		OffsetDirection.Y = 1.0f;
		break;
		
	default:
		break;
	}

	FVector TargetLocation = OffsetDirection * OffsetAmount;
	FRotator AlignmentRotation = (TargetLocation * -1).Rotation();
	
	MeshComponent->SetRelativeLocation(TargetLocation + FVector(0.0f, 0.0f, MeshBounds.Z));
	MeshComponent->SetRelativeRotation(AlignmentRotation);
}

void ATile::UpdateTileBox()
{
	TileBox->SetBoxExtent(FVector::ZeroVector);

	FVector Origin = FVector::ZeroVector;
	FVector NewBoxExtent = FVector::ZeroVector;
	GetActorBounds(true, Origin, NewBoxExtent);

	NewBoxExtent.Z = FMath::Clamp(NewBoxExtent.Z, 100.0f, std::numeric_limits<float>::max());
	
	TileBox->SetBoxExtent(NewBoxExtent);
	TileBox->SetRelativeLocation(FVector(0.0f, 0.0f, NewBoxExtent.Z));
}

void ATile::UpdateNextAttachArrow()
{
	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;
	
	if (FloorMeshAsset)
	{
		FVector FloorBounds = FloorMeshAsset->GetBounds().BoxExtent;

		switch (E_NextAttachLocation)
		{
			case ETileAttachLocation::Forward:
				TargetLocation.X = FloorBounds.X;
				break;
			case ETileAttachLocation::Left:
				TargetLocation.Y = -FloorBounds.Y;
				break;
			case ETileAttachLocation::Right:
				TargetLocation.Y = FloorBounds.Y;
				break;
		default:
			break;
		}
	}
	
	switch (E_NextAttachLocation)
	{
	case ETileAttachLocation::Forward:
		TargetRotation.Yaw = 0.0f;
		break;
	case ETileAttachLocation::Left:
		TargetRotation.Yaw = -90.0f;
		break;
	case ETileAttachLocation::Right:
		TargetRotation.Yaw = 90.0f;
		break;
	default:
		break;
	}

	NextAttachArrow->SetRelativeLocation(TargetLocation);
	NextAttachArrow->SetRelativeRotation(TargetRotation);
}

void ATile::UpdateLanes()
{
	LeftLaneSpline->ClearSplinePoints();
	MiddleLaneSpline->ClearSplinePoints();
	RightLaneSpline->ClearSplinePoints();
	
	FVector LeftLaneBegin = FVector::ZeroVector;
	FVector MiddleLaneBegin = FVector::ZeroVector;
	FVector RightLaneBegin = FVector::ZeroVector;

	FVector LeftLaneEnd = FVector::ZeroVector;
	FVector MiddleLaneEnd = FVector::ZeroVector;
	FVector RightLaneEnd = FVector::ZeroVector;

	FVector LeftLaneBeginLeaveTangent = FVector::ZeroVector;
	FVector LeftLaneEndArriveTangent = FVector::ZeroVector;
	FVector MiddleLaneBeginLeaveTangent = FVector::ZeroVector;
	FVector MiddleLaneEndArriveTangent = FVector::ZeroVector;
	FVector RightLaneBeginLeaveTangent = FVector::ZeroVector;
	FVector RightLaneEndArriveTangent = FVector::ZeroVector;
	
	if (FloorMeshAsset)
	{
		FVector FloorBounds = FloorMeshAsset->GetBounds().BoxExtent;

		float LeftLaneY = -FloorBounds.Y/2*LaneSpacingMultiplier;
		float RightLaneY = FloorBounds.Y/2*LaneSpacingMultiplier;
		float ZBuffer = FloorBounds.Z+10.0f;

		float Overhang = 0.0f;
		
		LeftLaneBegin = FVector(-FloorBounds.X - Overhang, LeftLaneY, ZBuffer);
		MiddleLaneBegin = FVector(-FloorBounds.X - Overhang, 0.0f, ZBuffer);
		RightLaneBegin = FVector(-FloorBounds.X - Overhang, RightLaneY, ZBuffer);

		if (E_NextAttachLocation != ETileAttachLocation::Forward)
		{
			float LeftLaneTurnX = 0.0f;
			float RightLaneTurnX = 0.0f;
			float LaneEndY = 0.0f;

			switch (E_NextAttachLocation)
			{
			case ETileAttachLocation::Left:
				LeftLaneTurnX = -FloorBounds.Y/2*LaneSpacingMultiplier;
				RightLaneTurnX = FloorBounds.Y/2*LaneSpacingMultiplier;
				LaneEndY = -FloorBounds.Y - Overhang;
				break;
				
			case ETileAttachLocation::Right:
				LeftLaneTurnX = FloorBounds.Y/2*LaneSpacingMultiplier;
				RightLaneTurnX = -FloorBounds.Y/2*LaneSpacingMultiplier;
				LaneEndY = FloorBounds.Y + Overhang;
				break;
				
			default:
				break;
			}
			
			LeftLaneEnd = FVector(LeftLaneTurnX, LaneEndY, ZBuffer);
			MiddleLaneEnd = FVector(0.0f, LaneEndY, ZBuffer);
			RightLaneEnd = FVector(RightLaneTurnX, LaneEndY, ZBuffer);

			LeftLaneBeginLeaveTangent = FVector((LeftLaneEnd.X - LeftLaneBegin.X) * 2.5, 0.0f, 0.0f);
			LeftLaneEndArriveTangent = FVector(0.0f, (LeftLaneEnd.Y - LeftLaneBegin.Y) * 2.5, 0.0f);

			MiddleLaneBeginLeaveTangent = FVector((MiddleLaneEnd.X - MiddleLaneBegin.X) * 2.5, 0.0f, 0.0f);
			MiddleLaneEndArriveTangent = FVector(0.0f, (MiddleLaneEnd.Y - MiddleLaneBegin.Y) * 2.5, 0.0f);

			RightLaneBeginLeaveTangent = FVector((RightLaneEnd.X - RightLaneBegin.X) * 2.5, 0.0f, 0.0f);
			RightLaneEndArriveTangent = FVector(0.0f, (RightLaneEnd.Y - RightLaneBegin.Y) * 2.5, 0.0f);

			goto PopulateSplines;
		}

		LeftLaneEnd = FVector(FloorBounds.X + Overhang, LeftLaneY, ZBuffer);
		MiddleLaneEnd = FVector(FloorBounds.X + Overhang, 0.0f, ZBuffer);
		RightLaneEnd = FVector(FloorBounds.X + Overhang, RightLaneY, ZBuffer);
	}

	PopulateSplines:
	
	LeftLaneSpline->AddSplineLocalPoint(LeftLaneBegin);
	MiddleLaneSpline->AddSplineLocalPoint(MiddleLaneBegin);
	RightLaneSpline->AddSplineLocalPoint(RightLaneBegin);

	LeftLaneSpline->AddSplineLocalPoint(LeftLaneEnd);
	MiddleLaneSpline->AddSplineLocalPoint(MiddleLaneEnd);
	RightLaneSpline->AddSplineLocalPoint(RightLaneEnd);
	
	if (E_NextAttachLocation != ETileAttachLocation::Forward)
	{
		LeftLaneSpline->SetTangentsAtSplinePoint(0, FVector(0.0f, 0.0f, 0.0f), LeftLaneBeginLeaveTangent, ESplineCoordinateSpace::Local);
		LeftLaneSpline->SetTangentsAtSplinePoint(1, LeftLaneEndArriveTangent, FVector(0.0f, 0.0f,0.0f), ESplineCoordinateSpace::Local);

		MiddleLaneSpline->SetTangentsAtSplinePoint(0, FVector(0.0f, 0.0f, 0.0f), MiddleLaneBeginLeaveTangent, ESplineCoordinateSpace::Local);
		MiddleLaneSpline->SetTangentsAtSplinePoint(1, MiddleLaneEndArriveTangent, FVector(0.0f, 0.0f,0.0f), ESplineCoordinateSpace::Local);

		RightLaneSpline->SetTangentsAtSplinePoint(0, FVector(0.0f, 0.0f, 0.0f), RightLaneBeginLeaveTangent, ESplineCoordinateSpace::Local);
		RightLaneSpline->SetTangentsAtSplinePoint(1, RightLaneEndArriveTangent, FVector(0.0f, 0.0f,0.0f), ESplineCoordinateSpace::Local);
	}
}
