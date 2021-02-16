// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASReferenceGameMode.h"
#include "GASReferenceCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGASReferenceGameMode::AGASReferenceGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
