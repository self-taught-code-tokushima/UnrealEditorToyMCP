// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "JsonObjectWrapper.h"

/**
 * Parameter definition for a command
 * Used to describe command parameters in tool lists
 */
struct FCommandParameter
{
	FString Name;
	FString Type;
	bool bRequired;
	FString Description;

	FCommandParameter(const FString& InName, const FString& InType, bool bInRequired, const FString& InDescription)
		: Name(InName)
		, Type(InType)
		, bRequired(bInRequired)
		, Description(InDescription)
	{
	}
};

/**
 * Base interface for all editor commands
 * Implements the Command pattern for extensible command handling
 */
class IEditorCommand
{
public:
	virtual ~IEditorCommand() = default;

	/**
	 * Get the command name (used in tool registration and execution)
	 * @return Command name (e.g., "ping", "get_actors_in_level")
	 */
	virtual FString GetName() const = 0;

	/**
	 * Get the command description (shown in tool lists)
	 * @return Human-readable description
	 */
	virtual FString GetDescription() const = 0;

	/**
	 * Get parameter definitions for this command
	 * @return Array of parameter definitions
	 */
	virtual TArray<FCommandParameter> GetParameters() const = 0;

	/**
	 * Execute the command with given parameters
	 * @param Params JSON object containing command parameters
	 * @return FJsonObjectWrapper with command result
	 */
	virtual FJsonObjectWrapper Execute(const TSharedPtr<FJsonObject>& Params) = 0;
};
