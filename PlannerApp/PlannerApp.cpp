
#include "PlanerApp.h"

int main()
{
    std::cout << "Connecting to the NPC Action Pipe..." << std::endl;

    // Example command (JSON format or any other structured format)
    std::string command = R"({"npcId": "12345", "action": "sit"})";

    command = "Hello from the planner app!";

    // Send the command to the pipe server
    SendCommand(command);

    return 0;
}

void SendCommand(const std::string& command)
{
    // Connect to the named pipe server
    HANDLE hPipe = CreateFileA(
        PIPE_NAME,              // Name of the pipe
        GENERIC_WRITE,          // Write access
        0,                      // No sharing
        NULL,                   // Default security attributes
        OPEN_EXISTING,          // Open existing pipe
        0,                      // Default attributes
        NULL);                  // No template file

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to named pipe. Error: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Sending message..." << std::endl;;

    // Send the command to the pipe
    DWORD bytesWritten;
    BOOL success = WriteFile(
        hPipe,                  // Handle to the pipe
        command.c_str(),        // Data to write
        (DWORD)command.size(),  // Number of bytes to write
        &bytesWritten,          // Number of bytes written
        NULL);                  // No overlapping

    if (!success) {
        std::cerr << "Failed to send command to pipe. Error: " << GetLastError() << std::endl;
    }
    else {
        std::cout << "Command sent successfully: " << command << std::endl;
    }

    // Close the pipe handle
    CloseHandle(hPipe);
}