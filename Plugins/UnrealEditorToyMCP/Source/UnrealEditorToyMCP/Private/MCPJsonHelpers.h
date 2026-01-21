#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "HttpServerResponse.h"
#include "MCPJsonStructs.h"

class FMCPJsonHelpers
{
public:
	// USTRUCT から JSON 文字列への変換
	template<typename StructType>
	static bool StructToJsonString(const StructType& Struct, FString& OutJsonString)
	{
		return FJsonObjectConverter::UStructToJsonObjectString(
			StructType::StaticStruct(),
			&Struct,
			OutJsonString,
			0, 0
		);
	}

	// JSON レスポンスの作成
	template<typename StructType>
	static TUniquePtr<FHttpServerResponse> CreateJsonResponse(
		const StructType& Struct,
		EHttpServerResponseCodes Code = EHttpServerResponseCodes::Ok)
	{
		FString JsonString;
		if (!StructToJsonString(Struct, JsonString))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize struct to JSON"));
			return CreateErrorResponse(TEXT("Internal server error"), EHttpServerResponseCodes::ServerError);
		}

		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(JsonString, TEXT("application/json"));
		Response->Code = Code;

		// CORS headers
		Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), {TEXT("http://localhost")});
		Response->Headers.Add(TEXT("Access-Control-Allow-Methods"), {TEXT("GET, POST, OPTIONS")});
		Response->Headers.Add(TEXT("Access-Control-Allow-Headers"), {TEXT("Content-Type")});

		return Response;
	}

	// エラーレスポンスの作成
	static TUniquePtr<FHttpServerResponse> CreateErrorResponse(
		const FString& ErrorMessage,
		EHttpServerResponseCodes Code = EHttpServerResponseCodes::BadRequest);

	// IEditorCommand から FMCPToolInfo への変換
	static FMCPToolInfo CommandToToolInfo(const TSharedPtr<class IEditorCommand>& Command, bool bIncludeParameters = true);

	// IEditorCommand 配列から FMCPToolInfo 配列への変換
	static TArray<FMCPToolInfo> CommandsToToolInfoArray(const TArray<TSharedPtr<class IEditorCommand>>& Commands, bool bIncludeParameters = true);
};
