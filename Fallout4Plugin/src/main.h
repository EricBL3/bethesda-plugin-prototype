#pragma once

#include <Windows.h>

#define PIPE_NAME "\\\\.\\pipe\\NPCActionPipe"

HANDLE CreatePipeInstance();
void WaitForClientConnection(HANDLE hPipe, OVERLAPPED& overlapped);
void HandlePipeCommunication(HANDLE hPipe);
void RunPipeServer();

void HandleIncomingMessage(const std::string& message);
