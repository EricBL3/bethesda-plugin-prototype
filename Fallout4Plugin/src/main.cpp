F4SEPluginLoad(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

    F4SE::GetMessagingInterface()->RegisterListener([](F4SE::MessagingInterface::Message* message) {
        if (message->type == F4SE::MessagingInterface::kGameLoaded)
            RE::ConsoleLog::GetSingleton()->PrintLine("Welcome to the Fallout 4 prototype plugin!");
    });

	return true;
}
