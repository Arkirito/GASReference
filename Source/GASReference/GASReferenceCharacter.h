// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "GASReferenceCharacter.generated.h"

UCLASS(config=Game)
class AGASReferenceCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UGASR_AbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const class UGASR_AttributeSet* AttributeSet;

public:	
	AGASReferenceCharacter();

	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void InitializeAttributes();
	void GiveAbilities();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DefaultAttributeSet;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	FGameplayTagContainer MovementBlockingTags;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	FGameplayTagContainer ViewBlockingTags;

	UFUNCTION()
	void OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle);

	UFUNCTION()
	void Shoot();

	UFUNCTION()
	void MeleeAttack();

	UFUNCTION(BlueprintImplementableEvent)
	void ApplyDamageToTarget(AActor* Target);

	void Die();

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	bool ApplyAffectToTarget(TSubclassOf<UGameplayEffect> Effect, AActor* Target);

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	bool CanMove() const;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	bool HasAnyViewBlockingTags() const;
	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MeleeAttackRadius = 200;

	float MeleeAttackRadiusSquared;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | Abilities | Effects")
	TSubclassOf<UGameplayEffect> GunshotDamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | Abilities | Effects")
	TSubclassOf<UGameplayEffect> MeleeDamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montages")
	UAnimMontage* GunshotMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montages")
	UAnimMontage* MeleeAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;
};

