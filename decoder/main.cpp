#include <iostream>
#include <csignal>
#include "cdecoder_server.h"


void Stop(int i)
{
	chen::g_decoder_server.stop();
	 
}

void RegisterSignal()
{
	signal(SIGINT, Stop);
	signal(SIGTERM, Stop);

}



int main(int argc, char* argv[])
{


	RegisterSignal();
	const char* config_filename = "server.cfg";
	const char* log_path = "./log";
	if (argc > 1)
	{
		config_filename = argv[1];
	}
	if (argc > 2)
	{
		log_path = argv[2];
	}

	bool init = chen::g_decoder_server.init(log_path, config_filename);

	if (init)
	{
		init = chen::g_decoder_server.Loop();
	}
	chen::g_decoder_server.Destroy();
	if (!init)
	{
		return 1;
	}


	return EXIT_SUCCESS;
}