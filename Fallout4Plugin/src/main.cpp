#include "main.h"

F4SEPluginLoad(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

    F4SE::GetMessagingInterface()->RegisterListener([](F4SE::MessagingInterface::Message* message) {
        if (message->type == F4SE::MessagingInterface::kGameLoaded)
        {
            RE::ConsoleLog::GetSingleton()->PrintLine("Welcome to the Fallout 4 prototype plugin!");

            RE::ConsoleLog::GetSingleton()->PrintLine("Starting pipe server...");
            std::thread pipeThread(RunPipeServer);
            pipeThread.detach();

        }
    });

	return true;
}

HANDLE CreatePipeInstance()
{
    HANDLE hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // Overlapped for async
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,                                       // Maximum instances
        0,                                       // Output buffer size
        0,                                       // Input buffer size
        0,                                       // Default timeout
        NULL                                     // Default security attributes
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        RE::ConsoleLog::GetSingleton()->PrintLine("Pipe creation failed!");
        return NULL;
    }

    RE::ConsoleLog::GetSingleton()->PrintLine("Pipe created!");

    return hPipe;
}

void WaitForClientConnection(HANDLE hPipe, OVERLAPPED& overlapped) {
    RE::ConsoleLog::GetSingleton()->PrintLine("Waiting for connection");

    // Initiate an asynchronous connection attempt
    BOOL connectSuccess = ConnectNamedPipe(hPipe, &overlapped);

    if (!connectSuccess) {
        DWORD error = GetLastError();
        if (error == ERROR_PIPE_CONNECTED) {
            // The client connected between CreateNamedPipe and ConnectNamedPipe
            RE::ConsoleLog::GetSingleton()->PrintLine("Pipe connected immediately!");
        }
        else if (error == ERROR_IO_PENDING) {
            // Wait for the connection to complete
            WaitForSingleObject(overlapped.hEvent, INFINITE);
            RE::ConsoleLog::GetSingleton()->PrintLine("Pipe connected (after waiting)!");
        }
        else {
            RE::ConsoleLog::GetSingleton()->PrintLine("Failed to connect to client!");
            CloseHandle(hPipe);
        }
    }
    else {
        RE::ConsoleLog::GetSingleton()->PrintLine("Pipe connected!");
    }
}

void HandlePipeCommunication(HANDLE hPipe) {
    char buffer[1024];

    while (true) {
        DWORD bytesRead;
        BOOL success = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);

        if (!success || bytesRead == 0) {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE) {
                RE::ConsoleLog::GetSingleton()->PrintLine("Client disconnected.");
                break; // Exit the loop to recreate the pipe for a new client
            }
            else {
                RE::ConsoleLog::GetSingleton()->PrintLine("ReadFile failed!");
                break; // Exit the loop to handle the error
            }
        }

        // Null-terminate the received data and process it
        buffer[bytesRead] = '\0';
        HandleIncomingMessage(std::string(buffer)); // Implement this to process the message
    }
}

void RunPipeServer() {
    while (true) {
        // Create a new named pipe instance
        HANDLE hPipe = CreatePipeInstance();
        if (hPipe == NULL) {
            return; // Exit if pipe creation fails
        }

        // Set up overlapped structure
        OVERLAPPED overlapped = {};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        // Wait for a client to connect
        WaitForClientConnection(hPipe, overlapped);

        // Handle communication with the client
        HandlePipeCommunication(hPipe);

        // Clean up the pipe instance
        CloseHandle(hPipe);
        RE::ConsoleLog::GetSingleton()->PrintLine("Pipe instance closed.");
    }
}

void HandleIncomingMessage(const std::string& message) {
    RE::ConsoleLog::GetSingleton()->PrintLine("Incoming message:");
    RE::ConsoleLog::GetSingleton()->PrintLine(message.c_str());
}
