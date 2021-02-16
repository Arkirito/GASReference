// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemPhysicsVolume.generated.h"

/**
 * 
 */
UCLASS()
class GASREFERENCE_API AAbilitySystemPhysicsVolume : public APhysicsVolume
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<class UGameplayEffect>> GameplayEffectsToApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	bool bDrawDebug = false;

public:

	AAbilitySystemPhysicsVolume();

	// Called when actor enters a volume
	virtual void ActorEnteredVolume(class AActor* Other) override;

	// Called when actor leaves a volume, Other can be NULL
	virtual void ActorLeavingVolume(class AActor* Other) override;

	virtual void Tick(float DeltaSeconds) override;

};
