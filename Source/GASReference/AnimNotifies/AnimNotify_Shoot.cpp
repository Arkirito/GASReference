// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_Shoot.h"
#include "GASReference/GASReferenceCharacter.h"

void UAnimNotify_Shoot::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGASReferenceCharacter* Character = MeshComp ? Cast<AGASReferenceCharacter>(MeshComp->GetOwner()) : nullptr;

	if (Character)
	{
		Character->Shoot();
	}
}