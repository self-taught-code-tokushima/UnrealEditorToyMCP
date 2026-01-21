#include "MCPJsonHelpers.h"
#include "Commands/IEditorCommand.h"

TUniquePtr<FHttpServerResponse> FMCPJsonHelpers::CreateErrorResponse(
	const FString& ErrorMessage,
	EHttpServerResponseCodes Code)
{
	FMCPErrorResponse ErrorResponse;
	ErrorResponse.error = ErrorMessage;
	return CreateJsonResponse(ErrorResponse, Code);
}

FMCPToolInfo FMCPJsonHelpers::CommandToToolInfo(const TSharedPtr<IEditorCommand>& Command, bool bIncludeParameters)
{
	FMCPToolInfo ToolInfo;

	if (!Command.IsValid())
	{
		return ToolInfo;
	}

	ToolInfo.name = Command->GetName();
	ToolInfo.description = Command->GetDescription();

	if (bIncludeParameters)
	{
		TArray<FCommandParameter> Parameters = Command->GetParameters();
		for (const FCommandParameter& Param : Parameters)
		{
			FMCPParameterInfo ParamInfo;
			ParamInfo.name = Param.Name;
			ParamInfo.type = Param.Type;
			ParamInfo.required = Param.bRequired;
			ParamInfo.description = Param.Description;
			ToolInfo.parameters.Add(ParamInfo);
		}
	}

	return ToolInfo;
}

TArray<FMCPToolInfo> FMCPJsonHelpers::CommandsToToolInfoArray(const TArray<TSharedPtr<IEditorCommand>>& Commands, bool bIncludeParameters)
{
	TArray<FMCPToolInfo> ToolInfoArray;

	for (const TSharedPtr<IEditorCommand>& Command : Commands)
	{
		if (Command.IsValid())
		{
			ToolInfoArray.Add(CommandToToolInfo(Command, bIncludeParameters));
		}
	}

	return ToolInfoArray;
}
