// Fill out your copyright notice in the Description page of Project Settings.

#include "Commands/EditorCommands.h"
#include "Commands/PythonOutputDevice.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

FEditorCommands::FEditorCommands()
{
}

FEditorCommands::~FEditorCommands()
{
}

FString FEditorCommands::Execute(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandType == TEXT("ping"))
	{
		return HandlePing(Params);
	}
	else if (CommandType == TEXT("get_actors_in_level"))
	{
		return HandleGetActorsInLevel(Params);
	}
	else if (CommandType == TEXT("execute_python"))
	{
		return HandleExecutePython(Params);
	}

	// Unknown command
	TSharedPtr<FJsonObject> ErrorResult = MakeShareable(new FJsonObject);
	ErrorResult->SetBoolField(TEXT("success"), false);
	ErrorResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown command: %s"), *CommandType));

	return SerializeJson(ErrorResult);
}

FString FEditorCommands::HandlePing(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
	Result->SetStringField(TEXT("message"), TEXT("pong"));
	return SerializeJson(Result);
}

FString FEditorCommands::HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ActorsArray;

	UWorld* EditorWorld = nullptr;
	if (GEditor)
	{
		EditorWorld = GEditor->GetEditorWorldContext().World();
	}

	if (EditorWorld)
	{
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(EditorWorld, AActor::StaticClass(), AllActors);

		for (AActor* Actor : AllActors)
		{
			if (!Actor) continue;

			TSharedPtr<FJsonObject> ActorJson = MakeShared<FJsonObject>();
			ActorJson->SetStringField(TEXT("name"), Actor->GetName());
			ActorJson->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

			// Location
			FVector Location = Actor->GetActorLocation();
			TSharedPtr<FJsonObject> LocationJson = MakeShared<FJsonObject>();
			LocationJson->SetNumberField(TEXT("x"), Location.X);
			LocationJson->SetNumberField(TEXT("y"), Location.Y);
			LocationJson->SetNumberField(TEXT("z"), Location.Z);
			ActorJson->SetObjectField(TEXT("location"), LocationJson);

			// Rotation
			FRotator Rotation = Actor->GetActorRotation();
			TSharedPtr<FJsonObject> RotationJson = MakeShared<FJsonObject>();
			RotationJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
			RotationJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
			RotationJson->SetNumberField(TEXT("roll"), Rotation.Roll);
			ActorJson->SetObjectField(TEXT("rotation"), RotationJson);

			// Scale
			FVector Scale = Actor->GetActorScale3D();
			TSharedPtr<FJsonObject> ScaleJson = MakeShared<FJsonObject>();
			ScaleJson->SetNumberField(TEXT("x"), Scale.X);
			ScaleJson->SetNumberField(TEXT("y"), Scale.Y);
			ScaleJson->SetNumberField(TEXT("z"), Scale.Z);
			ActorJson->SetObjectField(TEXT("scale"), ScaleJson);

			ActorsArray.Add(MakeShared<FJsonValueObject>(ActorJson));
		}

		Result->SetBoolField(TEXT("success"), true);
		Result->SetArrayField(TEXT("actors"), ActorsArray);
		Result->SetNumberField(TEXT("count"), ActorsArray.Num());
	}
	else
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("No editor world available"));
	}

	return SerializeJson(Result);
}

FString FEditorCommands::HandleExecutePython(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

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
	FString ScriptDir = EnsurePythonScriptDirectory();
	if (ScriptDir.IsEmpty())
	{
		Result->SetBoolField(TEXT("success"), false);
		Result->SetStringField(TEXT("error"), TEXT("Failed to create Python script directory"));
		return SerializeJson(Result);
	}

	// 5. Write script to file
	FString ScriptPath = GeneratePythonScriptPath(ScriptName);
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
	FString Output = OutputDevice.GetTrimmedOutput();
	bool bHasError = DetectPythonError(Output);

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

// Helper: Serialize JSON object to string
FString FEditorCommands::SerializeJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	return ResultString;
}

// Helper: Ensure Python script directory exists
FString FEditorCommands::EnsurePythonScriptDirectory()
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

// Helper: Generate Python script file path
FString FEditorCommands::GeneratePythonScriptPath(const FString& ScriptName)
{
	FString ScriptDir = EnsurePythonScriptDirectory();
	FString FileName = ScriptName.EndsWith(TEXT(".py")) ? ScriptName : ScriptName + TEXT(".py");
	return FPaths::Combine(ScriptDir, FileName);
}

// Helper: Detect Python errors in output
bool FEditorCommands::DetectPythonError(const FString& Output)
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
