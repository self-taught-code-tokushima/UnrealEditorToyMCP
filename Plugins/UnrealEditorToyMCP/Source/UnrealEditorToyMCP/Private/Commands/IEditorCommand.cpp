// Fill out your copyright notice in the Description page of Project Settings.

#include "IEditorCommand.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString IEditorCommand::SerializeJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	return ResultString;
}
