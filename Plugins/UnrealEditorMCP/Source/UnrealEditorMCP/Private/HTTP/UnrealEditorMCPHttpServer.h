// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IHttpRouter.h"

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

	// Response helpers
	static TUniquePtr<FHttpServerResponse> CreateJsonResponse(const FString& JsonContent, EHttpServerResponseCodes Code = EHttpServerResponseCodes::Ok);
	static TUniquePtr<FHttpServerResponse> CreateErrorResponse(const FString& Message, EHttpServerResponseCodes Code = EHttpServerResponseCodes::BadRequest);

	// HTTP infrastructure
	TSharedPtr<IHttpRouter> HttpRouter;
	FHttpRouteHandle ListToolsHandle;
	FHttpRouteHandle ExecuteToolHandle;
	FHttpRouteHandle StatusHandle;

	// Server state
	bool bIsRunning;
	uint32 ServerPort;
};
