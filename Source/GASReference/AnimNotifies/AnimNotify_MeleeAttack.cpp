// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_MeleeAttack.h"
#include "GASReference/GASReferenceCharacter.h"

void UAnimNotify_MeleeAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGASReferenceCharacter* Character = MeshComp ? Cast<AGASReferenceCharacter>(MeshComp->GetOwner()) : nullptr;

	if (Character)
	{
		Character->MeleeAttack();
	}
}