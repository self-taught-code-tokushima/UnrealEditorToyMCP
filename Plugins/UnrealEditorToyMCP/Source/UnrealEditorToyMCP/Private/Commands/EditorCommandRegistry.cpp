// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorCommandRegistry.h"

FEditorCommandRegistry::FEditorCommandRegistry()
{
}

FEditorCommandRegistry::~FEditorCommandRegistry()
{
	Commands.Empty();
}

void FEditorCommandRegistry::RegisterCommand(TSharedPtr<IEditorCommand> Command)
{
	if (!Command.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UnrealEditorMCP: Attempted to register invalid command"));
		return;
	}

	const FString CommandName = Command->GetName();
	if (Commands.Contains(CommandName))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnrealEditorMCP: Command '%s' is already registered, overwriting"), *CommandName);
	}

	Commands.Add(CommandName, Command);
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Registered command '%s'"), *CommandName);
}

TSharedPtr<IEditorCommand> FEditorCommandRegistry::GetCommand(const FString& CommandName) const
{
	const TSharedPtr<IEditorCommand>* FoundCommand = Commands.Find(CommandName);
	return FoundCommand ? *FoundCommand : nullptr;
}

TArray<TSharedPtr<IEditorCommand>> FEditorCommandRegistry::GetAllCommands() const
{
	TArray<TSharedPtr<IEditorCommand>> CommandArray;
	Commands.GenerateValueArray(CommandArray);
	return CommandArray;
}

bool FEditorCommandRegistry::HasCommand(const FString& CommandName) const
{
	return Commands.Contains(CommandName);
}

int32 FEditorCommandRegistry::GetCommandCount() const
{
	return Commands.Num();
}
