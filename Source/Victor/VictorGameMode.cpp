// Copyright Epic Games, Inc. All Rights Reserved.

#include "VictorGameMode.h"
#include "VictorCharacter.h"

AVictorGameMode::AVictorGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AVictorCharacter::StaticClass();	
}
