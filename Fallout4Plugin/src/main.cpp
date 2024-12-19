#include "main.h"

F4SEPluginLoad(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

    F4SE::GetMessagingInterface()->RegisterListener([](F4SE::MessagingInterface::Message* message) {
        if (message->type == F4SE::MessagingInterface::kGameLoaded)
        {
            RE::ConsoleLog::GetSingleton()->PrintLine("Welcome to the Fallout 4 prototype plugin!");

            RE::ConsoleLog::GetSingleton()->PrintLine("Starting pipe server...");
            std::thread pipeThread(StartPipeServer);
            pipeThread.detach();

        }
    });

	return true;
}

void StartPipeServer()
{
    HANDLE hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\NPCActionPipe",
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // Overlapped for async
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 0, 0, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        RE::ConsoleLog::GetSingleton()->PrintLine("Pipe creation failed!");
        return;
    }

    RE::ConsoleLog::GetSingleton()->PrintLine("Pipe created!");

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    RE::ConsoleLog::GetSingleton()->PrintLine("Waiting for connection");
    ConnectNamedPipe(hPipe, &overlapped);
    WaitForSingleObject(overlapped.hEvent, INFINITE); // Async connect

    RE::ConsoleLog::GetSingleton()->PrintLine("Pipe connected!");
    // Now handle communication
    while (true) {
        char buffer[1024];
        DWORD bytesRead;
        BOOL success = ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL);
        if (success) {
            buffer[bytesRead] = '\0'; // Null-terminate message
            HandleIncomingMessage(buffer); // Process received data
        }
    }

    CloseHandle(hPipe);
}

void HandleIncomingMessage(const std::string& message) {
    RE::ConsoleLog::GetSingleton()->PrintLine("Processing message: {%s}", message.c_str());
}
