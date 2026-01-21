// Fill out your copyright notice in the Description page of Project Settings.

#include "ExecutePythonCommand.h"
#include "PythonOutputDevice.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

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

FString FExecutePythonCommand::Execute(const TSharedPtr<FJsonObject>& Params)
{
	const TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

	// 1. Validate parameters
	if (!Params.IsValid())
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Missing parameters"));
		return SerializeJson(Result);
	}

	// 2. Get script_content (required)
	FString ScriptContent;
	if (!Params->TryGetStringField(TEXT("script_content"), ScriptContent) || ScriptContent.IsEmpty())
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Missing or empty 'script_content' parameter"));
		return SerializeJson(Result);
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
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Failed to create Python script directory"));
		return SerializeJson(Result);
	}

	// 5. Write a script to file
	const FString ScriptPath = GeneratePythonScriptPath(ScriptName);
	if (!FFileHelper::SaveStringToFile(ScriptContent, *ScriptPath))
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Failed to write script file"));
		return SerializeJson(Result);
	}

	// 6. Check GEditor availability
	if (!GEditor)
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Editor not available"));
		return SerializeJson(Result);
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("No active world"));
		return SerializeJson(Result);
	}

	// 7. Execute Python script via console command
	FPythonOutputDevice OutputDevice;
	FString PyScriptContent = FString::Printf(TEXT("py %s"), *ScriptContent);
	GEditor->Exec(World, *PyScriptContent, OutputDevice);

	// 8. Get output and detect errors
	const FString Output = OutputDevice.GetTrimmedOutput();
	const bool bHasError = DetectPythonError(Output);

	// 9. Build response
	Result->SetBoolField(TEXT("success"), !bHasError);
	Result->SetStringField(TEXT("output"), Output);
	Result->SetStringField(TEXT("script_path"), ScriptPath);

	if (bHasError)
	{
		Result->SetStringField(TEXT("error"), TEXT("Python script execution failed (see output)"));
	}

	return SerializeJson(Result);
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
