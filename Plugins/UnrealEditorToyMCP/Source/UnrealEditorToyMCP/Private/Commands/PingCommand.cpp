// Fill out your copyright notice in the Description page of Project Settings.

#include "PingCommand.h"

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

FString FPingCommand::Execute(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
	Result->SetStringField(TEXT("message"), TEXT("pong"));
	return SerializeJson(Result);
}
