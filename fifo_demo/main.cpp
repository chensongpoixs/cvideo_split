#include <iostream>

#include <string>



int main(int argc, char* argv[])
{


	std::string o_cmd = "v=0\r\no=0210011320001 0 0 IN IP4 172.20.16.3\r\ns= PlayMulticast\r\n";
	auto y_sdp_first_index = o_cmd.find("o=");
	auto y_sdp = o_cmd.substr(y_sdp_first_index);
	auto y_sdp_last_index = y_sdp.find("\r\n");
	std::string ssrc = y_sdp.substr(2, y_sdp_last_index - 1);



	return EXIT_SUCCESS;
}