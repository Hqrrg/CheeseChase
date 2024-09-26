// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CheeseChaseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

enum class ETileLane : uint8;

UCLASS(config=Game)
class ACheeseChaseCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ChooseLaneAction;

public:
	ACheeseChaseCharacter();

protected:
	virtual void BeginPlay();
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void ChooseLane(const FInputActionValue& Value);

	UFUNCTION()
	void Move();

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE class ATile* GetCurrentTile() const { return CurrentTile; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCurrentTile(class ATile* NewTile) { CurrentTile = NewTile; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE ETileLane GetMovementLane() const { return MovementLane; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMovementLane(ETileLane TileLane) { MovementLane = TileLane; }

private:
	FTimerHandle MovementTimerHandle;
	FTimerDelegate MovementTimerDelegate;

	UPROPERTY()
	class ATile* CurrentTile = nullptr;

	ETileLane MovementLane;
};

