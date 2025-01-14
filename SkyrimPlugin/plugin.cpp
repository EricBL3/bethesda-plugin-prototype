#include "logger.h"

class OurEventSink : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent> {
    OurEventSink() = default;
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;

public:

    static OurEventSink* GetSingleton() {
        static OurEventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* event,
        RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) {
        RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

        SKSE::log::info("Loading player from cell fully loaded event");

        if (!event || !player) {
            SKSE::log::warn("Player or event not found");
            return RE::BSEventNotifyControl::kContinue;
        }

        // Get the current cell the player entered
        RE::TESObjectCELL* currentCell = player->parentCell;
        SKSE::log::info("Loading current cell");

        if (currentCell) {
            RE::ConsoleLog::GetSingleton()->Print("Player entered cell: %s", currentCell->GetFullName());

            SKSE::log::info("Player entered cell: {}", currentCell->GetFullName());

            // Trigger your logic here
            HandleCellEntry(currentCell);
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void HandleCellEntry(RE::TESObjectCELL* cell) {
        // Example: Iterate through all NPCs in the cell

        if (!cell)
        {
            return;
        }

        SKSE::log::info("Checking cell references");

        cell->ForEachReference([](RE::TESObjectREFR& ref) -> RE::BSContainer::ForEachResult {
            if (ref.Is(RE::FormType::ActorCharacter)) {
                auto actor = ref.As<RE::Actor>();
                SKSE::log::info("reference is actor");
                if (actor && !actor->IsPlayerRef()) {
                    RE::ConsoleLog::GetSingleton()->Print("Found NPC: %s", actor->GetName());
                    SKSE::log::info("Found NPC: {}", actor->GetName());
                    // Iterate throught npc actions
                }
            }

            return RE::BSContainer::ForEachResult::kContinue;
        });

    }
};

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    SetupLog();

    // Add event sink
    auto* eventSink = OurEventSink::GetSingleton();

    // ScriptSource
    auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
    eventSourceHolder->AddEventSink<RE::TESCellFullyLoadedEvent>(eventSink);

    // register pipe
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
        {
            RE::ConsoleLog::GetSingleton()->Print("Welcome to the Skyrim prototype plugin!");

            RE::ConsoleLog::GetSingleton()->Print("Starting pipe server...");
            std::thread pipeThread(RunPipeServer);
            pipeThread.detach();

        }
    });


    return true;
}

#pragma region Pipes

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
        RE::ConsoleLog::GetSingleton()->Print("Pipe creation failed!");
        return NULL;
    }

    RE::ConsoleLog::GetSingleton()->Print("Pipe created!");

    return hPipe;
}

void WaitForClientConnection(HANDLE hPipe, OVERLAPPED& overlapped) {
    RE::ConsoleLog::GetSingleton()->Print("Waiting for connection");

    // Initiate an asynchronous connection attempt
    BOOL connectSuccess = ConnectNamedPipe(hPipe, &overlapped);

    if (!connectSuccess) {
        DWORD error = GetLastError();
        if (error == ERROR_PIPE_CONNECTED) {
            // The client connected between CreateNamedPipe and ConnectNamedPipe
            RE::ConsoleLog::GetSingleton()->Print("Pipe connected immediately!");
        }
        else if (error == ERROR_IO_PENDING) {
            // Wait for the connection to complete
            WaitForSingleObject(overlapped.hEvent, INFINITE);
            RE::ConsoleLog::GetSingleton()->Print("Pipe connected (after waiting)!");
        }
        else {
            RE::ConsoleLog::GetSingleton()->Print("Failed to connect to client!");
            CloseHandle(hPipe);
        }
    }
    else {
        RE::ConsoleLog::GetSingleton()->Print("Pipe connected!");
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
                RE::ConsoleLog::GetSingleton()->Print("Client disconnected.");
                break; // Exit the loop to recreate the pipe for a new client
            }
            else {
                RE::ConsoleLog::GetSingleton()->Print("ReadFile failed!");
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
        RE::ConsoleLog::GetSingleton()->Print("Pipe instance closed.");
    }
}

void HandleIncomingMessage(const std::string& message) {
    RE::ConsoleLog::GetSingleton()->Print("Incoming message:");
    RE::ConsoleLog::GetSingleton()->Print(message.c_str());

    ////NPC form id: 08001ECD
    //auto npcFormId = 0x08001ECD;
    ////Chair form id: 08001ECE
    ////Chair 2 form id: 000D8F57
    //auto chairFormID = 0x000D8F57;

    //if (message == "sit")
    //{

    //    auto npcForm = RE::TESForm::GetFormByID<RE::Actor>(npcFormId);
    //    auto furnitureForm = RE::TESForm::GetFormByID(chairFormID);
    //    if (!furnitureForm)
    //    {
    //        RE::ConsoleLog::GetSingleton()->Print("chair form not found!");
    //        return;
    //    }
    //    if (!npcForm)
    //    {
    //        RE::ConsoleLog::GetSingleton()->Print("npc form not found!");
    //        return;
    //    }

    //    RE::ConsoleLog::GetSingleton()->Print("Found form IDs!");

    //    auto npcActor = npcForm->As<RE::TESObjectREFR>();
    //    auto furnitureRef = furnitureForm->As<RE::TESObjectREFR>();

    //    if (npcActor && furnitureRef)
    //    {
    //        auto aiPackage = RE::AIProcess::CreateSitPackage(furnitureRef, npcActor);
    //        RE::ConsoleLog::GetSingleton()->Print("Will activate chair!");
    //        npcActor->Activate(furnitureRef, npcActor, nullptr, 1);
    //        npcActor->AddPackage();
    //        RE::ConsoleLog::GetSingleton()->Print("chair activated!");
    //    }
    //    else
    //    {
    //        if (!npcActor)
    //        {
    //            RE::ConsoleLog::GetSingleton()->Print("npc reference could not be found!");
    //        }
    //        if (!furnitureRef)
    //        {
    //            RE::ConsoleLog::GetSingleton()->Print("chair reference not found!");
    //        }
    //    }
    //}
}

#pragma endregion Pipes