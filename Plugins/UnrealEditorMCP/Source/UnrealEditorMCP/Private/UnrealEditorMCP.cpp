// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealEditorMCP.h"

#define LOCTEXT_NAMESPACE "FUnrealEditorMCPModule"

void FUnrealEditorMCPModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Module loaded"));
}

void FUnrealEditorMCPModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Module unloaded"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealEditorMCPModule, UnrealEditorMCP)