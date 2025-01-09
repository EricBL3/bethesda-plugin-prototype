#pragma once

// This file is required.

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <Windows.h>

#define PIPE_NAME "\\\\.\\pipe\\NPCActionPipe"

using namespace std::literals;

HANDLE CreatePipeInstance();
void WaitForClientConnection(HANDLE hPipe, OVERLAPPED& overlapped);
void HandlePipeCommunication(HANDLE hPipe);
void RunPipeServer();

void HandleIncomingMessage(const std::string& message);