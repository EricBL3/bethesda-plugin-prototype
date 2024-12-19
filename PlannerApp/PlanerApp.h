#pragma once

#include <windows.h>
#include <iostream>

#define PIPE_NAME "\\\\.\\pipe\\NPCActionPipe"

void SendCommand(const std::string& command);