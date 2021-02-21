// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASReferenceCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystem/GASR_AbilitySystemComponent.h"
#include "AbilitySystem/GASR_AttributeSet.h"
#include "AbilitySystem/GASR_GameplayAbility.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/CollisionProfile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include <GameplayEffectTypes.h>

//////////////////////////////////////////////////////////////////////////
// AGASReferenceCharacter

AGASReferenceCharacter::AGASReferenceCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	AbilitySystemComponent = CreateDefaultSubobject<UGASR_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &AGASReferenceCharacter::OnGameplayEffectAppliedToSelf);

	AttributeSet = CreateDefaultSubobject<UGASR_AttributeSet>("AttributeSet");

	HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AGASReferenceCharacter::OnHealthChanged);
	MaxWalkSpeedChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &AGASReferenceCharacter::OnMaxWalkSpeedChanged);
}

void AGASReferenceCharacter::BeginPlay()
{
	Super::BeginPlay();

	MeleeAttackRadiusSquared = MeleeAttackRadius * MeleeAttackRadius;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGASReferenceCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGASReferenceCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGASReferenceCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AGASReferenceCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("TurnRate", this, &AGASReferenceCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AGASReferenceCharacter::LookUpAtRate);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AGASReferenceCharacter::LookUpAtRate);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AGASReferenceCharacter::OnResetVR);
}

void AGASReferenceCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100, FString::Printf(TEXT("Health: %f"), Data.NewValue), nullptr, FColor::White, 5);

	if (Data.OldValue > 0 && Data.NewValue <= 0)
	{
		Die();
	}
}

void AGASReferenceCharacter::OnMaxWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}


UAbilitySystemComponent* AGASReferenceCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGASReferenceCharacter::InitializeAttributes()
{
	if (GetLocalRole() == ROLE_Authority && DefaultAttributeSet && AttributeSet)
	{
		ApplyAffectToTarget(DefaultAttributeSet, this);

		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetMaxMovementSpeed();
	}
}

void AGASReferenceCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (auto StartupAbility : DefaultAbilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility));
		}
	}
}

void AGASReferenceCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Server GAS Init
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitializeAttributes();
	GiveAbilities();
}

void AGASReferenceCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAttributes();
}

void AGASReferenceCharacter::OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle)
{
	
}

void AGASReferenceCharacter::Shoot()
{
	FVector ViewPoint;
	FRotator ViewRotation;
	UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraViewPoint(ViewPoint, ViewRotation);
	//FVector Direction = ViewRotation.Vector();
	FVector Direction = FollowCamera->GetForwardVector();
	ViewPoint = FollowCamera->GetComponentLocation();

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewPoint + Direction * 10000, ECollisionChannel::ECC_Camera);

	FVector WeaponHitDirection = Direction;

	if (HitResult.bBlockingHit)
	{
		WeaponHitDirection = HitResult.Location - GetActorLocation();

		DrawDebugSphere(GetWorld(), HitResult.Location, 25, 16, FColor::Red, false, 1, 0, 3);
	}
	FHitResult WeaponHitResult;

	WeaponHitDirection.Normalize();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(WeaponHitResult, GetActorLocation(), GetActorLocation() + WeaponHitDirection * 1000, ECollisionChannel::ECC_Camera, Params);

	if (WeaponHitResult.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), WeaponHitResult.Location, FColor::Red, false, 1, 0, 3);

		if (WeaponHitResult.Actor.Get())
		{
			if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(WeaponHitResult.GetActor()))
			{
				ApplyAffectToTarget(GunshotDamageEffect, WeaponHitResult.GetActor());
			}
		}
	}
	else
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WeaponHitDirection * 1000, FColor::Yellow, false, 1, 0, 3);
	}
}

void AGASReferenceCharacter::MeleeAttack()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	UBlackboardComponent* Blackboard = AIController ? AIController->GetBlackboardComponent() : nullptr;
	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(FName(TEXT("Target")))) : nullptr;

	if (Target && (FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) < MeleeAttackRadiusSquared))
	{
		ApplyAffectToTarget(MeleeDamageEffect, Target);
	}
}

void AGASReferenceCharacter::Die()
{
	GetMesh()->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AGASReferenceCharacter::IsAlive() const
{
	return AttributeSet ? AttributeSet->GetHealth() > 0 : true;
}

bool AGASReferenceCharacter::ApplyAffectToTarget(TSubclassOf<UGameplayEffect> Effect, AActor* Target)
{
	if (!Effect.Get()) return false;

	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}

		return true;
	}

	return false;
}

void AGASReferenceCharacter::OnResetVR()
{
	// If GASReference is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in GASReference.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AGASReferenceCharacter::TurnAtRate(float Rate)
{
	if (!HasAnyViewBlockingTags())
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AGASReferenceCharacter::LookUpAtRate(float Rate)
{
	if (!HasAnyViewBlockingTags())
	{
		// calculate delta for this frame from the rate information
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

bool AGASReferenceCharacter::CanMove() const
{
	if (AbilitySystemComponent)
	{
		return !AbilitySystemComponent->HasAnyMatchingGameplayTags(MovementBlockingTags);
	}

	return false;
}

void AGASReferenceCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && CanMove())
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AGASReferenceCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) && CanMove())
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

bool AGASReferenceCharacter::HasAnyViewBlockingTags() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(ViewBlockingTags);
	}

	return false;
}
