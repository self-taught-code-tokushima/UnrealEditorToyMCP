// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealEditorMCPHttpServer.h"
#include "HttpServerModule.h"
#include "Editor.h"                    // GEditor
#include "Engine/World.h"              // UWorld
#include "GameFramework/Actor.h"       // AActor
#include "Kismet/GameplayStatics.h"    // UGameplayStatics


FUnrealEditorMCPHttpServer::FUnrealEditorMCPHttpServer()
	: bIsRunning(false)
	  , ServerPort(0)
{
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
	const TSharedPtr<FJsonObject> ResponseJson = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	// Tool: ping
	{
		TSharedPtr<FJsonObject> ToolJson = MakeShared<FJsonObject>();
		ToolJson->SetStringField(TEXT("name"), TEXT("ping"));
		ToolJson->SetStringField(TEXT("description"), TEXT("Test server connectivity"));
		ToolJson->SetArrayField(TEXT("parameters"), TArray<TSharedPtr<FJsonValue>>());
		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolJson));
	}

	// Tool: get_actors_in_level
	{
		TSharedPtr<FJsonObject> ToolJson = MakeShared<FJsonObject>();
		ToolJson->SetStringField(TEXT("name"), TEXT("get_actors_in_level"));
		ToolJson->SetStringField(TEXT("description"), TEXT("Get all actors in the current editor level"));
		ToolJson->SetArrayField(TEXT("parameters"), TArray<TSharedPtr<FJsonValue>>());
		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolJson));
	}

	ResponseJson->SetArrayField(TEXT("tools"), ToolsArray);

	FString JsonString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);

	OnComplete(CreateJsonResponse(JsonString));
	return true;
}

bool FUnrealEditorMCPHttpServer::HandleExecuteTool(const FHttpServerRequest& Request,
                                                   const FHttpResultCallback& OnComplete) const
{
	// 1. Extract tool name from path: /mcp/tool/{name}
	FString RelativePath = Request.RelativePath.GetPath();
	FString CommandName;

	// Debug log to see what path we're receiving
	UE_LOG(LogTemp, Display, TEXT("UnrealEditorMCP HTTP: Received path: %s"), *RelativePath);

	// Parse tool name from path (handle different path formats)
	if (RelativePath.StartsWith(TEXT("/mcp/tool/")))
	{
		CommandName = RelativePath.RightChop(10); // Remove "/mcp/tool/"
	}
	else if (RelativePath.StartsWith(TEXT("/")))
	{
		CommandName = RelativePath.RightChop(1); // Remove leading "/"
	}
	else
	{
		CommandName = RelativePath; // Use as-is
	}

	if (CommandName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UnrealEditorMCP HTTP: Tool name is empty from path: %s"), *RelativePath);
		OnComplete(CreateErrorResponse(
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
			OnComplete(CreateErrorResponse(TEXT("Invalid JSON body"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// 3. Execute command on GameThread asynchronously
	AsyncTask(ENamedThreads::GameThread,
	          [this, CommandName, ParamsJson, OnComplete]()
	          {
		          FString ResultString;
		          if (CommandName == TEXT("ping"))
		          {
			          const TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
			          Result->SetStringField(TEXT("message"), TEXT("pong"));

			          const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
			          FJsonSerializer::Serialize(Result.ToSharedRef(), Writer);
		          }
		          else if (CommandName == TEXT("get_actors_in_level"))
		          {
			          TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
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

				          Result->SetArrayField(TEXT("actors"), ActorsArray);
				          Result->SetNumberField(TEXT("count"), ActorsArray.Num());
			          }
			          else
			          {
				          Result->SetStringField(TEXT("error"), TEXT("Failed to get editor world"));
			          }

			          TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
			          FJsonSerializer::Serialize(Result.ToSharedRef(), Writer);
		          }

		          // Build response
		          TSharedPtr<FJsonObject> ResponseJson = MakeShared<FJsonObject>();
		          TSharedPtr<FJsonObject> ResultJson;
		          TSharedRef<TJsonReader<>> ResultReader = TJsonReaderFactory<>::Create(ResultString);

		          if (FJsonSerializer::Deserialize(ResultReader, ResultJson))
		          {
			          ResponseJson->SetBoolField(TEXT("success"), true);
			          ResponseJson->SetStringField(TEXT("message"), TEXT("Command executed successfully"));
			          ResponseJson->SetObjectField(TEXT("data"), ResultJson);
		          }
		          else
		          {
			          ResponseJson->SetBoolField(TEXT("success"), false);
			          ResponseJson->SetStringField(TEXT("error"), TEXT("Failed to parse command result"));
		          }

		          FString JsonString;
		          TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		          FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);

		          // Call the completion callback
		          OnComplete(CreateJsonResponse(JsonString));
	          });

	return true;
}

bool FUnrealEditorMCPHttpServer::HandleStatus(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const
{
	TSharedPtr<FJsonObject> ResponseJson = MakeShared<FJsonObject>();
	ResponseJson->SetStringField(TEXT("status"), TEXT("running"));
	ResponseJson->SetNumberField(TEXT("httpPort"), ServerPort);
	ResponseJson->SetNumberField(TEXT("socketPort"), 55557); // Socket server port
	ResponseJson->SetStringField(TEXT("version"), TEXT("1.0.0"));
	ResponseJson->SetNumberField(TEXT("toolCount"), 2);  // ping + get_actors_in_level

	// Add tools list
	TArray<TSharedPtr<FJsonValue>> ToolsArray;
	{
		TSharedPtr<FJsonObject> ToolJson = MakeShared<FJsonObject>();
		ToolJson->SetStringField(TEXT("name"), TEXT("get_actors_in_level"));
		ToolJson->SetStringField(TEXT("description"), TEXT("Get all actors in the current editor level"));
		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolJson));
	}
	{
		TSharedPtr<FJsonObject> ToolJson = MakeShared<FJsonObject>();
		ToolJson->SetStringField(TEXT("name"), TEXT("ping"));
		ToolJson->SetStringField(TEXT("description"), TEXT("Test server connectivity"));
		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolJson));
	}
	ResponseJson->SetArrayField(TEXT("tools"), ToolsArray);

	// Add project and engine info
	ResponseJson->SetStringField(TEXT("projectName"), FApp::GetProjectName());
	ResponseJson->SetStringField(TEXT("engineVersion"), FEngineVersion::Current().ToString());

	FString JsonString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);

	OnComplete(CreateJsonResponse(JsonString));
	return true;
}

TUniquePtr<FHttpServerResponse> FUnrealEditorMCPHttpServer::CreateJsonResponse(
	const FString& JsonContent, EHttpServerResponseCodes Code)
{
	TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(JsonContent, TEXT("application/json"));
	Response->Code = Code;

	// Add CORS headers - restricted to localhost for security
	Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), {TEXT("http://localhost")});
	Response->Headers.Add(TEXT("Access-Control-Allow-Methods"), {TEXT("GET, POST, OPTIONS")});
	Response->Headers.Add(TEXT("Access-Control-Allow-Headers"), {TEXT("Content-Type")});

	return Response;
}

TUniquePtr<FHttpServerResponse> FUnrealEditorMCPHttpServer::CreateErrorResponse(
	const FString& Message, const EHttpServerResponseCodes Code)
{
	const TSharedPtr<FJsonObject> ErrorJson = MakeShared<FJsonObject>();
	ErrorJson->SetBoolField(TEXT("success"), false);
	ErrorJson->SetStringField(TEXT("error"), Message);

	FString JsonString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(ErrorJson.ToSharedRef(), Writer);

	return CreateJsonResponse(JsonString, Code);
}
