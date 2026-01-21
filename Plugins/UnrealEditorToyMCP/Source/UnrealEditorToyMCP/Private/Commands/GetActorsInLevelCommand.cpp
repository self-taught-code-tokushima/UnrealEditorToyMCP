// Fill out your copyright notice in the Description page of Project Settings.

#include "GetActorsInLevelCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "Dom/JsonValue.h"

FString FGetActorsInLevelCommand::GetName() const
{
	return TEXT("get_actors_in_level");
}

FString FGetActorsInLevelCommand::GetDescription() const
{
	return TEXT("Get all actors in the current editor level");
}

TArray<FCommandParameter> FGetActorsInLevelCommand::GetParameters() const
{
	// This command has no parameters
	return TArray<FCommandParameter>();
}

FString FGetActorsInLevelCommand::Execute(const TSharedPtr<FJsonObject>& Params)
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

			TSharedPtr<FJsonObject> ActorJson = BuildActorJson(Actor);
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

TSharedPtr<FJsonObject> FGetActorsInLevelCommand::BuildActorJson(AActor* Actor) const
{
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

	return ActorJson;
}
