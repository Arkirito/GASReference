// Fill out your copyright notice in the Description page of Project Settings.


#include "GASR_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GASR_AttributeSet.h"


UGASR_AttributeSet::UGASR_AttributeSet()
{

}

void UGASR_AttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASR_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASR_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASR_AttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

void UGASR_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASR_AttributeSet, Health, OldHealth);
}

void UGASR_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASR_AttributeSet, Stamina, OldStamina);
}

void UGASR_AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASR_AttributeSet, Mana, OldMana);
}