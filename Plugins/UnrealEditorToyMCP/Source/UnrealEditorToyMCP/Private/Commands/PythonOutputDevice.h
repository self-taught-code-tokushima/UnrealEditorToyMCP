// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

/**
 * Custom output device to capture Python script execution output
 * Used by EditorCommands to capture console output when executing Python scripts
 */
class FPythonOutputDevice : public FOutputDevice
{
public:
	FString Output;

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		Output += FString(V) + TEXT("\n");
	}

	void Clear()
	{
		Output.Empty();
	}

	FString GetTrimmedOutput() const
	{
		return Output.TrimStartAndEnd();
	}
};
