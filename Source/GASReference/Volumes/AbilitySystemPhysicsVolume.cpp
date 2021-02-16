// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemPhysicsVolume.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

AAbilitySystemPhysicsVolume::AAbilitySystemPhysicsVolume()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AAbilitySystemPhysicsVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		for (auto GameplayEffect : GameplayEffectsToApply)
		{
			FGameplayEffectContextHandle ContextHandle;
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(FGameplayEffectSpec(NewObject<UGameplayEffect>(this, GameplayEffect), ContextHandle));
		}
	}
}

void AAbilitySystemPhysicsVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		for (auto GameplayEffect : GameplayEffectsToApply)
		{
			FGameplayEffectContextHandle ContextHandle;
			AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(GameplayEffect, nullptr);
		}
	}
}

void AAbilitySystemPhysicsVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDrawDebug)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), GetBounds().BoxExtent, FColor::Red, false, 0, 0, 5);
	}
}
