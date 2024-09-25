// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"


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
}

FTransform ATile::GetNextAttachTransform() const
{
	return NextAttachArrow->GetComponentTransform();
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
	float OffsetAmount = ParentBounds.X - MeshBounds.X;

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
	
	FVector Origin = GetActorLocation();
	FVector NewBoxExtent = FVector(1.0f, 1.0f, 1.0f);
	GetActorBounds(true, Origin, NewBoxExtent);
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

	FVector LeftLaneTurn = FVector::ZeroVector;
	FVector MiddleLaneTurn = FVector::ZeroVector;
	FVector RightLaneTurn = FVector::ZeroVector;

	FVector LeftLaneEnd = FVector::ZeroVector;
	FVector MiddleLaneEnd = FVector::ZeroVector;
	FVector RightLaneEnd = FVector::ZeroVector;

	uint8 MaxSplinePoints = 2;
	
	if (FloorMeshAsset)
	{
		FVector FloorBounds = FloorMeshAsset->GetBounds().BoxExtent;

		float LeftLaneY = -FloorBounds.Y/2*LaneSpacingMultiplier;
		float RightLaneY = FloorBounds.Y/2*LaneSpacingMultiplier;
		float ZBuffer = FloorBounds.Z+10.0f;
		
		LeftLaneBegin = FVector(-FloorBounds.X, LeftLaneY, ZBuffer);
		MiddleLaneBegin = FVector(-FloorBounds.X, 0.0f, ZBuffer);
		RightLaneBegin = FVector(-FloorBounds.X, RightLaneY, ZBuffer);

		if (E_NextAttachLocation != ETileAttachLocation::Forward)
		{
			MaxSplinePoints = 3;
			
			float LeftLaneTurnX = 0.0f;
			float RightLaneTurnX = 0.0f;
			float LaneEndY = 0.0f;

			switch (E_NextAttachLocation)
			{
			case ETileAttachLocation::Left:
				LeftLaneTurnX = -FloorBounds.Y/2*LaneSpacingMultiplier;
				RightLaneTurnX = FloorBounds.Y/2*LaneSpacingMultiplier;
				LaneEndY = -FloorBounds.Y;
				break;
				
			case ETileAttachLocation::Right:
				LeftLaneTurnX = FloorBounds.Y/2*LaneSpacingMultiplier;
				RightLaneTurnX = -FloorBounds.Y/2*LaneSpacingMultiplier;
				LaneEndY = FloorBounds.Y;
				break;
				
			default:
				break;
			}

			LeftLaneTurn = FVector(LeftLaneTurnX, LeftLaneY, ZBuffer);
			MiddleLaneTurn = FVector(0.0f, 0.0f, ZBuffer);
			RightLaneTurn = FVector(RightLaneTurnX, RightLaneY, ZBuffer);
			
			LeftLaneEnd = FVector(LeftLaneTurnX, LaneEndY, ZBuffer);
			MiddleLaneEnd = FVector(0.0f, LaneEndY, ZBuffer);
			RightLaneEnd = FVector(RightLaneTurnX, LaneEndY, ZBuffer);

			goto PopulateSplines;
		}

		LeftLaneEnd = FVector(FloorBounds.X, LeftLaneY, ZBuffer);
		MiddleLaneEnd = FVector(FloorBounds.X, 0.0f, ZBuffer);
		RightLaneEnd = FVector(FloorBounds.X, RightLaneY, ZBuffer);
	}

	PopulateSplines:
	
	LeftLaneSpline->AddSplineLocalPoint(LeftLaneBegin);
	MiddleLaneSpline->AddSplineLocalPoint(MiddleLaneBegin);
	RightLaneSpline->AddSplineLocalPoint(RightLaneBegin);

	if (E_NextAttachLocation != ETileAttachLocation::Forward)
	{
		LeftLaneSpline->AddSplineLocalPoint(LeftLaneTurn);
		MiddleLaneSpline->AddSplineLocalPoint(MiddleLaneTurn);
		RightLaneSpline->AddSplineLocalPoint(RightLaneTurn);
	}

	LeftLaneSpline->AddSplineLocalPoint(LeftLaneEnd);
	MiddleLaneSpline->AddSplineLocalPoint(MiddleLaneEnd);
	RightLaneSpline->AddSplineLocalPoint(RightLaneEnd);


	for (uint8 Index = 0; Index < MaxSplinePoints; Index++)
	{
		LeftLaneSpline->SetSplinePointType(Index, ESplinePointType::Linear);
		MiddleLaneSpline->SetSplinePointType(Index, ESplinePointType::Linear);
		RightLaneSpline->SetSplinePointType(Index, ESplinePointType::Linear);
	}
}
