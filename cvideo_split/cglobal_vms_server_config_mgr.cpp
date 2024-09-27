/***********************************************************************************************
created: 		2024-01-25

author:			chensong

purpose:		camera

输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/
#include "cglobal_vms_server_config_mgr.h"
#include "ccfg.h"
#include "clog.h"
#include <json/json.h>
#include "clog.h"
#include "chttp_code.h"
#include <json/json.h>
#include "cvms_device.h"

namespace chen {
	static const char* vms_server_config_json_name = "vms_server_config.json";
	 
	cglobal_vms_server_config_mgr g_global_vms_server_config_mgr;
	cglobal_vms_server_config_mgr::~cglobal_vms_server_config_mgr()
	{
	}
	bool cglobal_vms_server_config_mgr::init()
	{
		_load_vms_server_config();
		return true;
	}
	void cglobal_vms_server_config_mgr::update(uint32 uDataTime)
	{
		if (m_data_type != EDataNone)
		{
			_write_vms_server_config();
			m_data_type = EDataNone;
		}
	}
	void cglobal_vms_server_config_mgr::destroy()
	{
		if (m_data_type != EDataNone)
		{
			_write_vms_server_config();
			m_data_type = EDataNone;
		}
	}
	cresult_vms_server_config cglobal_vms_server_config_mgr::handler_web_modify_vms_server_config(const cvms_server_config & server_config)
	{
	
		cresult_vms_server_config result;
		result.vms_config = server_config;
		m_vms_server_ip = server_config.vms_server_ip;
		m_vms_server_port = server_config.vms_server_port;
		m_vms_server_device_id = server_config.vms_server_device_id;
		m_video_split_device_id = server_config.video_split_device_id;
		m_user_name = server_config.user_name;
		m_pass_word = server_config.pass_word;

		NORMAL_EX_LOG("set vms server config");
		NORMAL_EX_LOG("[vms_server_ip = %s]", m_vms_server_ip.c_str());
		NORMAL_EX_LOG("[vms_server_port = %u]", m_vms_server_port);
		NORMAL_EX_LOG("[vms_server_device_id = %s]", m_vms_server_device_id.c_str());
		NORMAL_EX_LOG("[vms_video_split_id = %s]", m_video_split_device_id.c_str());
		NORMAL_EX_LOG("[vms_user_name = %s]", m_user_name.c_str());
		NORMAL_EX_LOG("[vms_pass_word = %s]", m_pass_word.c_str());
		m_data_type = EDataLoad;
		return result;
	}
	cresult_vms_server_config cglobal_vms_server_config_mgr::handler_web_get_vms_server_config()
	{
		cresult_vms_server_config vms_server_config;
		vms_server_config.vms_config.vms_server_ip			= m_vms_server_ip;
		vms_server_config.vms_config.vms_server_port		= m_vms_server_port;
		vms_server_config.vms_config.vms_server_device_id	= m_vms_server_device_id;
		vms_server_config.vms_config.video_split_device_id	= m_video_split_device_id;
		vms_server_config.vms_config.user_name				= m_user_name;
		vms_server_config.vms_config.pass_word				= m_pass_word;
		return vms_server_config;
	}

	uint32    cglobal_vms_server_config_mgr::handler_web_cmd_vms_server(uint32 cmd)
	{

		if (0 != cmd)
		{
			if (!g_vms_device_mgr.get_status())
			{
				g_vms_device_mgr.stop();
				g_vms_device_mgr.destroy();
				
			}
			else
			{
				WARNING_EX_LOG("vms server not run ing !!!");
			}
			return 0;
		}
		else
		{
			if (g_vms_device_mgr.get_status())
			{
				return 	g_vms_device_mgr.init(m_vms_server_ip, m_vms_server_port, m_vms_server_device_id, m_video_split_device_id, m_user_name, m_pass_word) == true ? 0 : 1;
			}
			else
			{
				WARNING_EX_LOG("vms server run ing !!! ");
				return 1;
			}
			
		}
	}


	void cglobal_vms_server_config_mgr::_load_vms_server_config()
	{
		std::string vms_server_config_name = g_cfg.get_string(ECI_DataPath) + "/" + vms_server_config_json_name;
		FILE* read_file_ptr = ::fopen(vms_server_config_name.c_str(), "rb");
		if (!read_file_ptr)
		{
			WARNING_EX_LOG("not open data file [%s] failed !!!", vms_server_config_name.c_str() );
			return;
		}

		// 数据的大小
		// 移动到文件末尾
		::fseek(read_file_ptr, 0, SEEK_END);
		// 获取当前位置，即文件大小
		uint32 size = ::ftell(read_file_ptr);
		::fseek(read_file_ptr, 0, SEEK_SET);


		std::string vms_server_config_data;
		vms_server_config_data.resize(size + 1);
		size_t read_size = ::fread((void *)vms_server_config_data.data(), 1, size, read_file_ptr);


		if (read_file_ptr)
		{
			::fclose(read_file_ptr);
			read_file_ptr = NULL;
		}
		/// parse json 
		_parse_json_data(vms_server_config_data);
	}
	void cglobal_vms_server_config_mgr::_parse_json_data(const std::string & vms_server_config_data)
	{

		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)vms_server_config_data.c_str(), (const char*)(vms_server_config_data.c_str() + vms_server_config_data.size()), data))
		{
			WARNING_EX_LOG("parse  [vms server config = %s] failed !!!", vms_server_config_data.c_str());
			//std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());

			return;
		}

		/*

		[
		{ "address": "192.168.1.34"},
		{"address": "192.168.1.35"}

		]
		*/

		if (!data.isObject())
		{
			WARNING_EX_LOG("vms server config data not is arrays [%s] !!!", data.asCString());
			return;
		}
		if (!data.isMember("vms_server_ip") || !data["vms_server_ip"].isString())
		{
			WARNING_EX_LOG("vms server config not find vms server ip  !!!");
			
		}
		else
		{
			m_vms_server_ip = data["vms_server_ip"].asString();
		}
		if (!data.isMember("vms_server_port") || !data["vms_server_port"].isUInt64())
		{
			WARNING_EX_LOG("vms server config not find vms server port  !!!");

		}
		else
		{
			m_vms_server_port = data["vms_server_port"].asUInt64();
		}


		if (!data.isMember("vms_server_device_id") || !data["vms_server_device_id"].isString())
		{
			WARNING_EX_LOG("vms server config not find vms server device id  !!!");

		}
		else
		{
			m_vms_server_device_id = data["vms_server_device_id"].asString();
		}

		if (!data.isMember("video_split_device_id") || !data["video_split_device_id"].isString())
		{
			WARNING_EX_LOG("vms server config not find video_split_device_id  !!!");

		}
		else
		{
			m_video_split_device_id = data["video_split_device_id"].asString();
		}

		if (!data.isMember("user_name") || !data["user_name"].isString())
		{
			WARNING_EX_LOG("vms server config not find user_name  !!!");

		}
		else
		{
			m_user_name = data["user_name"].asString();
		}

		if (!data.isMember("pass_word") || !data["pass_word"].isString())
		{
			WARNING_EX_LOG("vms server config not find pass_word  !!!");

		}
		else
		{
			m_pass_word = data["pass_word"].asString();
		}
	}


	void cglobal_vms_server_config_mgr::_write_vms_server_config()
	{
		Json::Value value;

		value["vms_server_ip"] = m_vms_server_ip;
		value["vms_server_port"] = m_vms_server_port;
		value["vms_server_device_id"] = m_vms_server_device_id;
		value["video_split_device_id"] = m_video_split_device_id;
		value["user_name"] = m_user_name;
		value["pass_word"] = m_pass_word;
		Json::StyledWriter swriter;
		std::string str = swriter.write(value);

		if (str.length() < 1)
		{
			WARNING_EX_LOG("not find data json --> [%s]", str.c_str());
			return;
		}
		//TODO@chensong 2024-02-06创建对应目录
		boost::filesystem::create_directories(g_cfg.get_string(ECI_DataPath));
		std::string camera_name = g_cfg.get_string(ECI_DataPath) + "/" + vms_server_config_json_name;
		FILE* write_file_ptr = ::fopen(camera_name.c_str(), "wb+");
		if (!write_file_ptr)
		{

			WARNING_EX_LOG("not open data file [%s] failed !!!", camera_name.c_str());
			return;
		}

		::fwrite(str.c_str(), 1, str.length(), write_file_ptr);
		::fflush(write_file_ptr);
		::fclose(write_file_ptr);
		write_file_ptr = NULL;
	}

	void cglobal_vms_server_config_mgr::_sync_save()
	{
		m_data_type = EDataLoad;
	}
}