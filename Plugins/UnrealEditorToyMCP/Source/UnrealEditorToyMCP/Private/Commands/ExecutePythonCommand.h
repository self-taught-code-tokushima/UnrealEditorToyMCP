// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEditorCommand.h"

/**
 * ExecutePython command - Executes Python scripts in the Unreal Editor
 * Parameters:
 *   - script_content (required): Python script content to execute
 *   - script_name (optional): Script name for logging (auto-generated if not provided)
 */
class FExecutePythonCommand : public IEditorCommand
{
public:
	virtual ~FExecutePythonCommand() override = default;

	// IEditorCommand interface
	virtual FString GetName() const override;
	virtual FString GetDescription() const override;
	virtual TArray<FCommandParameter> GetParameters() const override;
	virtual FString Execute(const TSharedPtr<FJsonObject>& Params) override;

private:
	/**
	 * Ensure Python script directory exists
	 * @return Directory path, or empty string on failure
	 */
	FString EnsurePythonScriptDirectory() const;

	/**
	 * Generate full path for Python script file
	 * @param ScriptName Script name (without or with .py extension)
	 * @return Full file path
	 */
	FString GeneratePythonScriptPath(const FString& ScriptName) const;

	/**
	 * Detect if Python output contains errors
	 * @param Output Output string from Python execution
	 * @return True if error detected, false otherwise
	 */
	bool DetectPythonError(const FString& Output) const;
};
