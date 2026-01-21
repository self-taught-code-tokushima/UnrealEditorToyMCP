#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "MCPJsonStructs.generated.h"

// パラメータ情報
USTRUCT()
struct FMCPParameterInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString type;

	UPROPERTY()
	bool required = false;

	UPROPERTY()
	FString description;
};

// ツール情報
USTRUCT()
struct FMCPToolInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString description;

	UPROPERTY()
	TArray<FMCPParameterInfo> parameters;
};

// GET /mcp/tools のレスポンス
USTRUCT()
struct FMCPToolsListResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMCPToolInfo> tools;
};

// POST /mcp/tool/{name} のレスポンス
USTRUCT()
struct FMCPCommandResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool success = true;

	UPROPERTY()
	FString message;

	UPROPERTY()
	FString error;

	UPROPERTY()
	FJsonObjectWrapper data;
};

// GET /mcp/status のレスポンス
USTRUCT()
struct FMCPStatusResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString status;

	UPROPERTY()
	int32 httpPort = 0;

	UPROPERTY()
	int32 socketPort = 0;

	UPROPERTY()
	FString version;

	UPROPERTY()
	int32 toolCount = 0;

	UPROPERTY()
	TArray<FMCPToolInfo> tools;

	UPROPERTY()
	FString projectName;

	UPROPERTY()
	FString engineVersion;
};

// エラーレスポンス
USTRUCT()
struct FMCPErrorResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool success = false;

	UPROPERTY()
	FString error;
};
