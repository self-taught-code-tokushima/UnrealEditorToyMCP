// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealEditorMCPHttpServer.h"
#include "Commands/EditorCommandRegistry.h"
#include "Commands/PingCommand.h"
#include "Commands/GetActorsInLevelCommand.h"
#include "Commands/ExecutePythonCommand.h"
#include "HttpServerModule.h"
#include "Editor.h"                    // GEditor
#include "Engine/World.h"              // UWorld
#include "GameFramework/Actor.h"       // AActor
#include "MCPJsonHelpers.h"            // JSON helper functions


FUnrealEditorMCPHttpServer::FUnrealEditorMCPHttpServer()
	: bIsRunning(false)
	  , ServerPort(0)
{
	// Initialize command registry
	CommandRegistry = MakeUnique<FEditorCommandRegistry>();

	// Register all commands
	CommandRegistry->RegisterCommand(MakeShared<FPingCommand>());
	CommandRegistry->RegisterCommand(MakeShared<FGetActorsInLevelCommand>());
	CommandRegistry->RegisterCommand(MakeShared<FExecutePythonCommand>());

	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: Registered %d commands"), CommandRegistry->GetCommandCount());
}

FUnrealEditorMCPHttpServer::~FUnrealEditorMCPHttpServer()
{
	Stop();
}


bool FUnrealEditorMCPHttpServer::Start(uint32 Port)
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnrealEditorMCP: HTTP Server already running"));
		return false;
	}

	ServerPort = Port;

	// Get HTTP server module
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();

	// Get HTTP router for the specified port
	HttpRouter = HttpServerModule.GetHttpRouter(ServerPort);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP: Failed to get HTTP router for port %d"), ServerPort);
		return false;
	}

	// Setup routes
	SetupRoutes();

	// Start the listeners
	HttpServerModule.StartAllListeners();

	bIsRunning = true;

	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: HTTP Server started on http://localhost:%d"), ServerPort);
	UE_LOG(LogTemp, Display, TEXT("  GET  /mcp/tools         - List available tools"));
	UE_LOG(LogTemp, Display, TEXT("  POST /mcp/tool/{name}   - Execute a tool"));
	UE_LOG(LogTemp, Display, TEXT("  GET  /mcp/status        - Server status"));

	return true;
}

void FUnrealEditorMCPHttpServer::Stop()
{
	if (!bIsRunning)
	{
		return;
	}

	if (HttpRouter.IsValid())
	{
		if (ListToolsHandle.IsValid())
		{
			HttpRouter->UnbindRoute(ListToolsHandle);
		}
		if (ExecuteToolHandle.IsValid())
		{
			HttpRouter->UnbindRoute(ExecuteToolHandle);
		}
		if (StatusHandle.IsValid())
		{
			HttpRouter->UnbindRoute(StatusHandle);
		}

		HttpRouter.Reset();
	}

	bIsRunning = false;
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP: HTTP Server stopped"));
}

void FUnrealEditorMCPHttpServer::SetupRoutes()
{
	if (!HttpRouter.IsValid())
	{
		return;
	}

	// GET /mcp/tools - List all available tools
	ListToolsHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/tools")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FUnrealEditorMCPHttpServer::HandleListTools)
	);

	// POST /mcp/tool/* - Execute a tool (wildcard path)
	ExecuteToolHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/tool")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FUnrealEditorMCPHttpServer::HandleExecuteTool)
	);

	// GET /mcp/status - Server status
	StatusHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/status")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FUnrealEditorMCPHttpServer::HandleStatus)
	);
}

bool FUnrealEditorMCPHttpServer::HandleListTools(const FHttpServerRequest& Request,
                                                 const FHttpResultCallback& OnComplete) const
{
	FMCPToolsListResponse Response;
	Response.tools = FMCPJsonHelpers::CommandsToToolInfoArray(CommandRegistry->GetAllCommands(), true);

	OnComplete(FMCPJsonHelpers::CreateJsonResponse(Response));
	return true;
}

bool FUnrealEditorMCPHttpServer::HandleExecuteTool(const FHttpServerRequest& Request,
                                                   const FHttpResultCallback& OnComplete) const
{
	// 1. Extract the tool name from a path: /mcp/tool/{name}
	FString RelativePath = Request.RelativePath.GetPath();
	FString CommandName;

	// Debug log to see what path we're receiving
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP HTTP: Received path: %s"), *RelativePath);

	// Parse tool name from a path (handle different path formats)
	if (RelativePath.StartsWith(TEXT("/mcp/tool/")))
	{
		CommandName = RelativePath.RightChop(10); // Remove "/mcp/tool/"
	}
	else if (RelativePath.StartsWith(TEXT("/")))
	{
		CommandName = RelativePath.RightChop(1); // Remove the leading "/"
	}
	else
	{
		CommandName = RelativePath; // Use as-is
	}

	if (CommandName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP HTTP: Tool name is empty from path: %s"), *RelativePath);
		OnComplete(FMCPJsonHelpers::CreateErrorResponse(
			TEXT("Tool name not specified. Use POST /mcp/tool/{toolname}"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP HTTP: Executing tool: %s"), *CommandName);

	// 2. Parse JSON body
	TSharedPtr<FJsonObject> ParamsJson = MakeShared<FJsonObject>();
	if (Request.Body.Num() > 0)
	{
		TArray<uint8> NullTerminatedBody = Request.Body;
		NullTerminatedBody.Add(0);
		const FString BodyString = UTF8_TO_TCHAR(reinterpret_cast<const char*>(NullTerminatedBody.GetData()));

		if (TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyString); !FJsonSerializer::Deserialize(Reader, ParamsJson))
		{
			UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP HTTP: Failed to parse JSON body: %s"), *BodyString);
			OnComplete(FMCPJsonHelpers::CreateErrorResponse(TEXT("Invalid JSON body"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// 3. Check if the command exists
	if (!CommandRegistry->HasCommand(CommandName))
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP HTTP: Unknown command: %s"), *CommandName);
		OnComplete(FMCPJsonHelpers::CreateErrorResponse(
			FString::Printf(TEXT("Unknown command: %s"), *CommandName),
			EHttpServerResponseCodes::NotFound));
		return true;
	}

	// 4. Execute command on GameThread asynchronously
	AsyncTask(ENamedThreads::GameThread,
	          [this, CommandName, ParamsJson, OnComplete]()
	          {
		          // Get command from registry
		          TSharedPtr<IEditorCommand> Command = CommandRegistry->GetCommand(CommandName);
		          if (!Command.IsValid())
		          {
			          FMCPCommandResponse ErrorResponse;
			          ErrorResponse.success = false;
			          ErrorResponse.error = TEXT("Command not found in registry");
			          OnComplete(FMCPJsonHelpers::CreateJsonResponse(ErrorResponse));
			          return;
		          }

		          // Execute command (returns FJsonObjectWrapper directly)
		          FJsonObjectWrapper ResultWrapper = Command->Execute(ParamsJson);

		          // Build response
		          FMCPCommandResponse Response;
		          Response.success = true;
		          Response.message = TEXT("Command executed successfully");
		          Response.data = ResultWrapper;

		          // Call the completion callback
		          OnComplete(FMCPJsonHelpers::CreateJsonResponse(Response));
	          });

	return true;
}

bool FUnrealEditorMCPHttpServer::HandleStatus(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const
{
	FMCPStatusResponse Response;
	Response.status = TEXT("running");
	Response.httpPort = ServerPort;
	Response.socketPort = 55557;
	Response.version = TEXT("1.0.0");
	Response.toolCount = CommandRegistry->GetCommandCount();
	Response.tools = FMCPJsonHelpers::CommandsToToolInfoArray(CommandRegistry->GetAllCommands(), false);
	Response.projectName = FApp::GetProjectName();
	Response.engineVersion = FEngineVersion::Current().ToString();

	OnComplete(FMCPJsonHelpers::CreateJsonResponse(Response));
	return true;
}
