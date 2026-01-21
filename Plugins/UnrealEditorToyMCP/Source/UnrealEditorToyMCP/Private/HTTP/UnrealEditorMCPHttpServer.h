// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IHttpRouter.h"

class FEditorCommandRegistry;

class FUnrealEditorMCPHttpServer
{
public:
	FUnrealEditorMCPHttpServer();
	~FUnrealEditorMCPHttpServer();

	// Server lifecycle
	bool Start(uint32 Port = 3000);
	void Stop();
	bool IsRunning() const { return bIsRunning; }
	uint32 GetPort() const { return ServerPort; }

private:
	// Route setup
	void SetupRoutes();

	// Endpoint handlers
	bool HandleListTools(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;
	bool HandleExecuteTool(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;
	bool HandleStatus(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;

	// HTTP infrastructure
	TSharedPtr<IHttpRouter> HttpRouter;
	FHttpRouteHandle ListToolsHandle;
	FHttpRouteHandle ExecuteToolHandle;
	FHttpRouteHandle StatusHandle;

	// Command registry
	TUniquePtr<FEditorCommandRegistry> CommandRegistry;

	// Server state
	bool bIsRunning;
	uint32 ServerPort;
};
