#include <iostream>
#include <signal.h>
#include "csip_robot.h"

#include <thread>
#include <chrono>
void Stop(int i)
{ 
	chen::g_sip_robot.stop();
}

void RegisterSignal()
{
	signal(SIGINT, Stop);
	signal(SIGTERM, Stop);

}
int main(int argc, char* argv[])
{
#if 0
#include <Windows.h>
	while (1)
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		// 转换为毫秒
		uint64_t time = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
		time /= 10000; // 转换为秒

		std::cout << "Hardware timestamp (milliseconds since January 1, 1601): " << time << std::endl;
		std::cout << " time --> " << ::time(NULL) << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
#endif //
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
	bool init = chen::g_sip_robot.init(log_path, config_filename);

	if (init)
	{
		init = chen::g_sip_robot.Loop();
	}
	chen::g_sip_robot.Destroy();
	return EXIT_SUCCESS;
}