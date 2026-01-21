// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEditorCommand.h"

class AActor;

/**
 * GetActorsInLevel command - Retrieves all actors in the current editor level
 * Returns actor information including name, class, location, rotation, and scale
 */
class FGetActorsInLevelCommand : public IEditorCommand
{
public:
	virtual ~FGetActorsInLevelCommand() override = default;

	// IEditorCommand interface
	virtual FString GetName() const override;
	virtual FString GetDescription() const override;
	virtual TArray<FCommandParameter> GetParameters() const override;
	virtual FString Execute(const TSharedPtr<FJsonObject>& Params) override;

private:
	/**
	 * Build JSON object for a single actor
	 * @param Actor Actor to convert to JSON
	 * @return JSON object containing actor information
	 */
	TSharedPtr<FJsonObject> BuildActorJson(AActor* Actor) const;
};
