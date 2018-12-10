// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SafetyFirstGameMode.h"
#include "SafetyFirstPawn.h"

ASafetyFirstGameMode::ASafetyFirstGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ASafetyFirstPawn::StaticClass();
}

