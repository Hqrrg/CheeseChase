// Copyright Epic Games, Inc. All Rights Reserved.

#include "CheeseChaseCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Tile.h"
#include "Components/SplineComponent.h"


ACheeseChaseCharacter::ACheeseChaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
}

void ACheeseChaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetMovementLane(ETileLane::Middle);

	UWorld* World = GetWorld();

	if (World)
	{
		MovementTimerDelegate.BindUFunction(this, FName("Move"));
		World->GetTimerManager().SetTimer(MovementTimerHandle, MovementTimerDelegate, 0.001f, true);
	}
}

void ACheeseChaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Choosing Lane
		EnhancedInputComponent->BindAction(ChooseLaneAction, ETriggerEvent::Triggered, this, &ACheeseChaseCharacter::ChooseLane);
	}
}

void ACheeseChaseCharacter::ChooseLane(const FInputActionValue& Value)
{
	int8 Direction = Value.Get<FInputActionValue::Axis1D>();
	
	uint8 CurrentMovementLaneIndex = static_cast<uint8>(MovementLane);
	uint8 NewMovementLaneIndex = CurrentMovementLaneIndex + Direction;
	
	ETileLane NewLane = MovementLane;

	switch (NewMovementLaneIndex)
	{
	case 0:
		NewLane = ETileLane::Left;
		break;
		
	case 1:
		NewLane = ETileLane::Middle;
		break;
		
	case 2:
		NewLane = ETileLane::Right;
		break;
		
	default:
		break;
	}

	SetMovementLane(NewLane);
}

void ACheeseChaseCharacter::Move()
{
	if (!CurrentTile) return;

	FVector ActorLocation = GetActorLocation();
	USplineComponent* MovementSpline = CurrentTile->GetLaneSpline(MovementLane);
	
	float DistanceAlong = MovementSpline->GetDistanceAlongSplineAtLocation(ActorLocation, ESplineCoordinateSpace::World);
	FVector TargetLocation = MovementSpline->GetWorldLocationAtDistanceAlongSpline(DistanceAlong + 100.0f);
	FRotator TargetRotation = MovementSpline->GetWorldRotationAtDistanceAlongSpline(DistanceAlong);

	FVector Direction = (TargetLocation - ActorLocation).GetSafeNormal();
	
	AddMovementInput(Direction, 1);
	SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
}
