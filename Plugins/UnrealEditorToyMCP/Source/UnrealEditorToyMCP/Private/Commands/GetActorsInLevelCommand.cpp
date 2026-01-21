// Fill out your copyright notice in the Description page of Project Settings.

#include "GetActorsInLevelCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "JsonObjectConverter.h"
#include "MCPJsonStructs.h"

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

FJsonObjectWrapper FGetActorsInLevelCommand::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FGetActorsInLevelCommandResponse Response;

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

			FMCPActorInfo ActorInfo = BuildActorInfo(Actor);
			Response.actors.Add(ActorInfo);
		}

		Response.success = true;
		Response.count = Response.actors.Num();
	}
	else
	{
		Response.success = false;
		Response.error = TEXT("No editor world available");
	}

	FJsonObjectWrapper Wrapper;
	Wrapper.JsonObject = FJsonObjectConverter::UStructToJsonObject(Response);
	return Wrapper;
}

FMCPActorInfo FGetActorsInLevelCommand::BuildActorInfo(AActor* Actor) const
{
	FMCPActorInfo ActorInfo;
	ActorInfo.name = Actor->GetName();
	ActorInfo.className = Actor->GetClass()->GetName();

	// Location
	FVector Location = Actor->GetActorLocation();
	ActorInfo.location.x = Location.X;
	ActorInfo.location.y = Location.Y;
	ActorInfo.location.z = Location.Z;

	// Rotation
	FRotator Rotation = Actor->GetActorRotation();
	ActorInfo.rotation.pitch = Rotation.Pitch;
	ActorInfo.rotation.yaw = Rotation.Yaw;
	ActorInfo.rotation.roll = Rotation.Roll;

	// Scale
	FVector Scale = Actor->GetActorScale3D();
	ActorInfo.scale.x = Scale.X;
	ActorInfo.scale.y = Scale.Y;
	ActorInfo.scale.z = Scale.Z;

	return ActorInfo;
}
