// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Editor command executor
 * Handles execution of various editor commands requested via HTTP
 */
class FEditorCommands
{
public:
	FEditorCommands();
	~FEditorCommands();

	// Main command dispatcher
	FString Execute(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// Command handlers
	FString HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params);
	FString HandlePing(const TSharedPtr<FJsonObject>& Params);
	FString HandleExecutePython(const TSharedPtr<FJsonObject>& Params);

	// Helper methods
	FString EnsurePythonScriptDirectory();
	FString GeneratePythonScriptPath(const FString& ScriptName);
	bool DetectPythonError(const FString& Output);
	FString SerializeJson(const TSharedPtr<FJsonObject>& JsonObject);
};
