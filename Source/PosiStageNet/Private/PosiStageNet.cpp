// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#include "PosiStageNet.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "PSN/psn_lib.hpp"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogPSN);
#define LOCTEXT_NAMESPACE "FPosiStageNetModule"

void FPosiStageNetModule::StartupModule()
{
}

void FPosiStageNetModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPosiStageNetModule, PosiStageNet)
