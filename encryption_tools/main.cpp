#include  <ostream>
#include <iostream>
#include "cmd5.h"


int main(int argc, char* argv[])
{

	
	std::string key = argv[1];
	printf("input key = [%s]\n", key.c_str());
	std::string   md5_name = key.substr(0, key.length() / 2);
	std::string   re_md5_name = key.substr((key.length() / 2), key.length() / 2);
	md5_name += "chensong20240719";

	re_md5_name += "chensong2024";

	std::string new_md5_name = chen::md5::md5_hash_hex(md5_name.c_str());
	std::string new_re_md5_name = chen::md5::md5_hash_hex(re_md5_name.c_str());

	std::string out_key = new_md5_name + new_re_md5_name;

	printf("output key = [%s]\n", out_key.c_str());


	return 0;
}