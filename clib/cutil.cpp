/***********************************************************************************************
    created:         2019-03-06
    
    author:            chensong
                    
    purpose:        ccrypto
************************************************************************************************/
#include "cutil.h"
#include <sstream>
#include <string>

namespace chen
{
	namespace cutil
	{

		static const char HEX[16] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'a', 'b',
			'c', 'd', 'e', 'f'
		};

		std::string get_hex_str(const void *_buf, int num)
		{
			std::string str;
			str.reserve(num << 1);
			const unsigned char* buf = (const unsigned char*)_buf;

			unsigned char tmp;
			for (int i = 0; i < num; ++i)
			{
				tmp = buf[i];
				str.append(1, HEX[tmp / 16]);
				str.append(1, HEX[tmp % 16]);
			}
			return str;
		}
	}

	namespace path_util
	{
		std::string convert_path(const std::string& path)
		{
			std::string result_path = path;
			//char p ;
			for (size_t i = 0; i < result_path.length(); ++i)
			{
				if (result_path.at(i) == '\\')
				{
					//p = '/';
					result_path[i] = '/';
				}
				 
			}
			return result_path;
		}


		std::string parent_path(const std::string& path)
		{
			std::string result_path;
			for (size_t i = path.length() - 2; i >= 0; --i)
			{
				if (path.at(i) == '/' || path.at(i) == '\\')
				{
					result_path = path.substr(0, i);
					break;
				}
			}
			return result_path;
		}
	}
}
