// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEditorCommand.h"

/**
 * Registry for editor commands
 * Manages command registration and lookup using the Registry pattern
 */
class FEditorCommandRegistry
{
public:
	FEditorCommandRegistry();
	~FEditorCommandRegistry();

	/**
	 * Register a new command
	 * @param Command Shared pointer to the command instance
	 */
	void RegisterCommand(TSharedPtr<IEditorCommand> Command);

	/**
	 * Get a command by name
	 * @param CommandName Name of the command to retrieve
	 * @return Shared pointer to the command, or nullptr if not found
	 */
	TSharedPtr<IEditorCommand> GetCommand(const FString& CommandName) const;

	/**
	 * Get all registered commands
	 * @return Array of all command instances
	 */
	TArray<TSharedPtr<IEditorCommand>> GetAllCommands() const;

	/**
	 * Check if a command exists
	 * @param CommandName Name of the command to check
	 * @return True if command exists, false otherwise
	 */
	bool HasCommand(const FString& CommandName) const;

	/**
	 * Get the number of registered commands
	 * @return Number of commands
	 */
	int32 GetCommandCount() const;

private:
	// Command storage: CommandName -> Command instance
	TMap<FString, TSharedPtr<IEditorCommand>> Commands;
};
