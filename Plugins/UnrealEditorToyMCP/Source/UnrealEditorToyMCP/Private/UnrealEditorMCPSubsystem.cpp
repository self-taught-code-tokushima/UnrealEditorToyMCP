// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealEditorMCPSubsystem.h"
#include "HTTP/UnrealEditorMCPHttpServer.h"

#define MCP_HTTP_SERVER_PORT 3000

UUnrealEditorMCPSubsystem::UUnrealEditorMCPSubsystem()
{
}

UUnrealEditorMCPSubsystem::~UUnrealEditorMCPSubsystem()
{
}

void UUnrealEditorMCPSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Initializing"));

	// Start HTTP server
	StartHttpServer();
}

void UUnrealEditorMCPSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Shutting down"));
	StopHttpServer();
}

// Start HTTP server
void UUnrealEditorMCPSubsystem::StartHttpServer()
{
	if (HttpServer.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UnrealEditorMCP: HTTP Server already running"));
		return;
	}

	HttpServer = MakeShared<FUnrealEditorMCPHttpServer>();

	if (!HttpServer->Start(MCP_HTTP_SERVER_PORT))
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP: Failed to start HTTP Server on port %d"), MCP_HTTP_SERVER_PORT);
		HttpServer.Reset();
	}
}

// Stop HTTP server
void UUnrealEditorMCPSubsystem::StopHttpServer()
{
	if (HttpServer.IsValid())
	{
		HttpServer->Stop();
		HttpServer.Reset();
	}
}