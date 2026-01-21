// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEditorCommand.h"

/**
 * Ping command - Tests server connectivity
 * Simple command with no parameters that returns "pong"
 */
class FPingCommand : public IEditorCommand
{
public:
	virtual ~FPingCommand() override = default;

	// IEditorCommand interface
	virtual FString GetName() const override;
	virtual FString GetDescription() const override;
	virtual TArray<FCommandParameter> GetParameters() const override;
	virtual FString Execute(const TSharedPtr<FJsonObject>& Params) override;
};
