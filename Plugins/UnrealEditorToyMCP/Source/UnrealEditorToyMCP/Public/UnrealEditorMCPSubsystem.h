// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UnrealEditorMCPSubsystem.generated.h"

class FUnrealEditorMCPHttpServer;

UCLASS()
class UNREALEDITORMCP_API UUnrealEditorMCPSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:
	UUnrealEditorMCPSubsystem();
	virtual ~UUnrealEditorMCPSubsystem() override;

	// UEditorSubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// HTTP Server
	TSharedPtr<FUnrealEditorMCPHttpServer> HttpServer;

	// HTTP Server functions
	void StartHttpServer();
	void StopHttpServer();
};