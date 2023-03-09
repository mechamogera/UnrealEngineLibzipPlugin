// Copyright Epic Games, Inc. All Rights Reserved.

#include "LibzipArchiverModule.h"
#include "Core.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FLibzipArchiverModule"

void FLibzipArchiverModule::StartupModule()
{
}

void FLibzipArchiverModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLibzipArchiverModule, LibzipArchiver)
