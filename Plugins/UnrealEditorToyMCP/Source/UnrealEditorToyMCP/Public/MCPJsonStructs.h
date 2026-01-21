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

// ============================================================================
// Command レスポンス用の構造体
// ============================================================================

// Vector3 (Location, Scale など用)
USTRUCT()
struct FMCPVector3
{
	GENERATED_BODY()

	UPROPERTY()
	double x = 0.0;

	UPROPERTY()
	double y = 0.0;

	UPROPERTY()
	double z = 0.0;
};

// Rotator (Rotation 用)
USTRUCT()
struct FMCPRotator
{
	GENERATED_BODY()

	UPROPERTY()
	double pitch = 0.0;

	UPROPERTY()
	double yaw = 0.0;

	UPROPERTY()
	double roll = 0.0;
};

// Actor 情報
USTRUCT()
struct FMCPActorInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString className;

	UPROPERTY()
	FMCPVector3 location;

	UPROPERTY()
	FMCPRotator rotation;

	UPROPERTY()
	FMCPVector3 scale;
};

// ping コマンドのレスポンス
USTRUCT()
struct FPingCommandResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString message;
};

// get_actors_in_level コマンドのレスポンス
USTRUCT()
struct FGetActorsInLevelCommandResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool success = true;

	UPROPERTY()
	TArray<FMCPActorInfo> actors;

	UPROPERTY()
	int32 count = 0;

	UPROPERTY()
	FString error;
};

// execute_python コマンドのレスポンス
USTRUCT()
struct FExecutePythonCommandResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool success = true;

	UPROPERTY()
	FString output;

	UPROPERTY()
	FString script_path;

	UPROPERTY()
	FString error;
};
