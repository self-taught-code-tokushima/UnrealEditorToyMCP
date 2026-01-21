// Fill out your copyright notice in the Description page of Project Settings.

#include "PingCommand.h"
#include "JsonObjectConverter.h"
#include "MCPJsonStructs.h"

FString FPingCommand::GetName() const
{
	return TEXT("ping");
}

FString FPingCommand::GetDescription() const
{
	return TEXT("Test server connectivity");
}

TArray<FCommandParameter> FPingCommand::GetParameters() const
{
	// Ping command has no parameters
	return TArray<FCommandParameter>();
}

FJsonObjectWrapper FPingCommand::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FPingCommandResponse Response;
	Response.message = TEXT("pong");

	FJsonObjectWrapper Wrapper;
	Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
	return Wrapper;
}
