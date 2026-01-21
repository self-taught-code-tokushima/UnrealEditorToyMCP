// Fill out your copyright notice in the Description page of Project Settings.

#include "ExecutePythonCommand.h"
#include "PythonOutputDevice.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "MCPJsonStructs.h"

FString FExecutePythonCommand::GetName() const
{
	return TEXT("execute_python");
}

FString FExecutePythonCommand::GetDescription() const
{
	return TEXT("Execute a Python script in the Unreal Editor");
}

TArray<FCommandParameter> FExecutePythonCommand::GetParameters() const
{
	TArray<FCommandParameter> Parameters;
	Parameters.Add(FCommandParameter(
		TEXT("script_content"),
		TEXT("string"),
		true,
		TEXT("Python script content to execute")
	));
	Parameters.Add(FCommandParameter(
		TEXT("script_name"),
		TEXT("string"),
		false,
		TEXT("Optional script name (auto-generated if not provided)")
	));
	return Parameters;
}

FJsonObjectWrapper FExecutePythonCommand::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FExecutePythonCommandResponse Response;

	// 1. Validate parameters
	if (!Params.IsValid())
	{
		Response.success = false;
		Response.error = TEXT("Missing parameters");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	// 2. Get script_content (required)
	FString ScriptContent;
	if (!Params->TryGetStringField(TEXT("script_content"), ScriptContent) || ScriptContent.IsEmpty())
	{
		Response.success = false;
		Response.error = TEXT("Missing or empty 'script_content' parameter");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	// 3. Get script_name (optional, auto-generate if not provided)
	FString ScriptName;
	if (!Params->TryGetStringField(TEXT("script_name"), ScriptName) || ScriptName.IsEmpty())
	{
		FDateTime Now = FDateTime::Now();
		ScriptName = FString::Printf(TEXT("script_%s"), *Now.ToString(TEXT("%Y%m%d_%H%M%S")));
	}

	// 4. Ensure directory exists
	const FString ScriptDir = EnsurePythonScriptDirectory();
	if (ScriptDir.IsEmpty())
	{
		Response.success = false;
		Response.error = TEXT("Failed to create Python script directory");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	// 5. Write a script to file
	const FString ScriptPath = GeneratePythonScriptPath(ScriptName);
	if (!FFileHelper::SaveStringToFile(ScriptContent, *ScriptPath))
	{
		Response.success = false;
		Response.error = TEXT("Failed to write script file");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	// 6. Check GEditor availability
	if (!GEditor)
	{
		Response.success = false;
		Response.error = TEXT("Editor not available");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		Response.success = false;
		Response.error = TEXT("No active world");

		FJsonObjectWrapper Wrapper;
		Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
		return Wrapper;
	}

	// 7. Execute Python script via console command
	FPythonOutputDevice OutputDevice;
	FString PyScriptContent = FString::Printf(TEXT("py %s"), *ScriptContent);
	GEditor->Exec(World, *PyScriptContent, OutputDevice);

	// 8. Get output and detect errors
	const FString Output = OutputDevice.GetTrimmedOutput();
	const bool bHasError = DetectPythonError(Output);

	// 9. Build response
	Response.success = !bHasError;
	Response.output = Output;
	Response.script_path = ScriptPath;

	if (bHasError)
	{
		Response.error = TEXT("Python script execution failed (see output)");
	}

	FJsonObjectWrapper Wrapper;
	Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
	return Wrapper;
}

FString FExecutePythonCommand::EnsurePythonScriptDirectory() const
{
	FString ProjectContentDir = FPaths::ProjectContentDir();
	FString ScriptDir = FPaths::Combine(ProjectContentDir, TEXT("Python"), TEXT("Scripts"));

	if (!IFileManager::Get().DirectoryExists(*ScriptDir))
	{
		if (!IFileManager::Get().MakeDirectory(*ScriptDir, true))
		{
			UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP: Failed to create directory: %s"), *ScriptDir);
			return FString();
		}
		UE_LOG(LogTemp, Log, TEXT("UnrealEditorMCP: Created Python script directory: %s"), *ScriptDir);
	}

	return ScriptDir;
}

FString FExecutePythonCommand::GeneratePythonScriptPath(const FString& ScriptName) const
{
	FString ScriptDir = EnsurePythonScriptDirectory();
	FString FileName = ScriptName.EndsWith(TEXT(".py")) ? ScriptName : ScriptName + TEXT(".py");
	return FPaths::Combine(ScriptDir, FileName);
}

bool FExecutePythonCommand::DetectPythonError(const FString& Output) const
{
	return Output.Contains(TEXT("Traceback")) ||
		   Output.Contains(TEXT("SyntaxError")) ||
		   Output.Contains(TEXT("NameError")) ||
		   Output.Contains(TEXT("TypeError")) ||
		   Output.Contains(TEXT("ValueError")) ||
		   Output.Contains(TEXT("ImportError")) ||
		   Output.Contains(TEXT("AttributeError")) ||
		   Output.Contains(TEXT("KeyError")) ||
		   Output.Contains(TEXT("IndexError")) ||
		   Output.Contains(TEXT("RuntimeError"));
}
