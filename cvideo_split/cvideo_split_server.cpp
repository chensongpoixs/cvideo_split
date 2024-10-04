/***********************************************************************************************
			created: 		2019-05-01

			author:			chensong

			purpose:		ccfg

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


#include "cvideo_split_server.h"
#include "clog.h"
#include "ctime_api.h"
#include "ccamera_info_mgr.h"
#include "ctime_elapse.h"
#include "cvideo_split_info_mgr.h"
#include "cvideo_split_mgr.h"
#include "cwebsocket_wan_server.h"
#include "clib_util.h"
#include "cclient_msg_dispatch.h"
#include "csystem_info.h"
#include "crandom.h"
#include "chardware_info.h"
#include "cmd5.h"
#include "cglobal_vms_port_mgr.h"
#include "cvms_device.h"
#include "cvms_msg_dispath.h"
#include "cglobal_vms_server_config_mgr.h"
#include "cvms_device.h"

namespace chen {


	cvideo_split_server g_video_split_server;

	std::vector<uint32> g_gpu_index;
	

	std::string    g_all_gpu_name;


	static std::vector<uint32> get_gpu_count()
	{
		std::vector<uint32> gpu_indexs;
		boost::process::ipstream p2;
		std::string outstr;

		bool ret = false;
		CUresult ret1;
		ret1 = cuInit(0);
		int device_count = 0;
		cuDeviceGetCount(&device_count);
		NORMAL_EX_LOG("cuda count = %u", device_count);
		std::thread reader([&p2, &gpu_indexs]
		{
			std::string line;
			while (std::getline(p2, line))
			{
				gpu_indexs.push_back(1);
				g_all_gpu_name += line;
			}
		});

		boost::process::child c(
			"nvidia-smi  --list-gpus",
			(boost::process::std_out & boost::process::std_err) > p2  //redirect both to one file
			//boost::process::std_in <  //read input from file
		);


		//c.wait();
		if (reader.joinable())
		{
			reader.join();
		}
		c.terminate();
		c.exit_code();
		return gpu_indexs;
	}
	bool cvideo_split_server::init(const char* log_path, const char* config_file)
	{
		printf("Log init ...\n");
		if (!LOG::init(log_path, "video_split"))
		{
			return false;
		}
		SYSTEM_LOG("config init ...");
		if (!g_cfg.init(config_file))
		{
			return false;
		}
		LOG::set_level(static_cast<ELogLevelType>(g_cfg.get_uint32(ECI_LogLevel)));
		ctime_base_api::set_time_zone(g_cfg.get_int32(ECI_TimeZone));
		ctime_base_api::set_time_adjust(g_cfg.get_int32(ECI_TimeAdjust));
		
		
		g_gpu_index = get_gpu_count();
		if (!_check_auth_info())
		{
			return false;
		}
		 
		
		SYSTEM_LOG("gpu info size = [%u]", g_gpu_index.size());


		if (!g_vms_msg_dispatch.init())
		{
			return false;
		}
		if (!g_global_vms_server_config_mgr.init())
		{
			return false;
		}

		if (!g_global_vms_port_mgr.init())
		{
			return false;
		}
		// check data path
		
		/*NORMAL_EX_LOG("uuid = [%s]", c_rand.rand_str(32).c_str());
		NORMAL_EX_LOG("uuid = [%s]", c_rand.rand_str(32).c_str());
		NORMAL_EX_LOG("uuid = [%s]", c_rand.rand_str(32).c_str());*/
	//	return false;
		// load camera data 
		SYSTEM_LOG("Load camera_list data ...");
		if (!g_camera_info_mgr.init())
		{
			return false;
		}
		SYSTEM_LOG("load camera_list data OK !!!");

		SYSTEM_LOG("Load video_split_list data ...");
		if (!g_video_split_info_mgr.init())
		{
			return false;
		}
		SYSTEM_LOG("Load video split list data OK !!!");

		SYSTEM_LOG("video split mgr ... ");
		if (!g_video_split_mgr.init())
		{
			return false;
		}
		SYSTEM_LOG("video split mgr init OK !!!");


		SYSTEM_LOG("vms Device init ...");
		/*if (!g_vms_device_mgr.init())
		{
			return false;
		}*/
		SYSTEM_LOG("vms device init OK!!!");

		SYSTEM_LOG("Web Server API init ...");
		if (!g_web_http_api_mgr.init())
		{
			return false;
		}
		SYSTEM_LOG("Web Server API startup ...");
		g_web_http_api_mgr.startup();
		SYSTEM_LOG("Web Server API startup OK ...");
		SYSTEM_LOG("dispatch init ...");

		if (!g_client_msg_dispatch.init())
		{
			return false;
		}
		SYSTEM_LOG("client_msg dispatch init OK !!!");
		if (g_cfg.get_uint32(ECI_OpenWebSocket))
		{
			SYSTEM_LOG("websocket wan server  init ...");
			if (!g_websocket_wan_server.init())
			{
				return false;
			}
			SYSTEM_LOG("websocket wan server  startup ...");
			if (!g_websocket_wan_server.startup())
			{
				return false;
			}
		}
		else
		{
			SYSTEM_LOG("not open websocket !!!");
		}
		
		SYSTEM_LOG("VideoSplit  server init ok");

		return true;
		return true;
	}

	bool cvideo_split_server::Loop()
	{
		static const uint32 TICK_TIME = 100;
		//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È´ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½
		//NORMAL_EX_LOG("");
		ctime_elapse time_elapse;
		uint32 uDelta = 0;
		while (!m_stoped)
		{
			uDelta += time_elapse.get_elapse();
			if (g_cfg.get_uint32(ECI_OpenWebSocket))
			{
				g_websocket_wan_server.update(uDelta);
			}
			g_vms_device_mgr.update(uDelta);
			g_global_vms_server_config_mgr.update(uDelta);
			g_global_vms_port_mgr.update(uDelta);
			g_http_queue_mgr.update(); 
			g_camera_info_mgr.update(uDelta);
			g_video_split_info_mgr.update(uDelta);
			g_video_split_mgr.udpate(uDelta);

			uDelta = time_elapse.get_elapse();

			if (uDelta < TICK_TIME)
			{
				csleep(TICK_TIME - uDelta);
			}
		}

		SYSTEM_LOG("Leave main loop");

		return true;
	}

	void cvideo_split_server::Destroy()
	{
		g_global_vms_server_config_mgr.destroy();
		g_websocket_wan_server.shutdown();
		g_websocket_wan_server.destroy();
		SYSTEM_LOG("g_wan_server Destroy OK!!!");
		g_web_http_api_mgr.destroy();
		SYSTEM_LOG("Web Server Destroy OK !!!");
		g_video_split_mgr.destroy();
		SYSTEM_LOG("video split mgr destroy OK !!!");

		g_camera_info_mgr.destroy();
		SYSTEM_LOG("camera_list info mgr destroy OK !!!");
		g_video_split_info_mgr.destroy();
		SYSTEM_LOG("video_split_info mgr destroy OK !!!");
	 	g_client_msg_dispatch.destroy();
		SYSTEM_LOG("msg dispath destroy OK !!!"); 

		g_vms_device_mgr.destroy();
		SYSTEM_LOG("vms device msg destroy OK !!!");

		g_global_vms_port_mgr.destroy();
		SYSTEM_LOG("global vms port destroy OK !!!");



		g_cfg.destroy();
		LOG::destroy();
		printf(" VideoSplit server Destroy End OK !!!\n");
	}

	void cvideo_split_server::stop()
	{
		m_stoped = true;
	}

	bool cvideo_split_server::_check_auth_info()
	{

		static std::set<std::string>   g_auth_info = {"93sf014wp1817n6n8439d18zp8s057uv", "i2a04651s42156x22nkc9eu9s70u20o9", "83g6102thw18100qb45o223y75584436"};


		//NORMAL_EX_LOG("gpu name = [%s]", g_all_gpu_name.c_str());
		//NORMAL_EX_LOG("wifi name = [%s]", chen::hardware_info::network_info().c_str());
		std::string all_name = g_all_gpu_name + chen::hardware_info::network_info();
		std::string md5_name = md5::md5_hash_hex(all_name.c_str());
		//NORMAL_EX_LOG("all_name = %s, md5 name = [%s]", all_name.c_str(), md5_name.c_str());
		std::string re_all_name = chen::hardware_info::network_info() + g_all_gpu_name;
		std::string re_md5_name = md5::md5_hash_hex(re_all_name.c_str());
		//NORMAL_EX_LOG("re_all_name = %s, re_md5 name = [%s]", re_all_name.c_str(), re_md5_name.c_str());
		std::string all_md5_cr_name = md5_name + re_md5_name;
		NORMAL_EX_LOG("key = [%s]", all_md5_cr_name.c_str());


		md5_name += "chensong20240719";

		re_md5_name += "chensong2024";

		std::string new_md5_name =  md5::md5_hash_hex(md5_name.c_str());
		std::string new_re_md5_name =  md5::md5_hash_hex(re_md5_name.c_str());



		std::string config_name = g_cfg.get_string(ECI_AuthPass);
		std::string   pre_config_name = config_name.substr(0, config_name.length()/ 2);
		std::string   hot_config_name = config_name.substr((config_name.length()/2) , config_name.length()/ 2);



		//NORMAL_EX_LOG("0_config_name  = %s, 1_config_name = [%s]", pre_config_name.c_str(), hot_config_name.c_str());

		if (pre_config_name == new_md5_name && new_re_md5_name == hot_config_name)
		{
			return true;
		}

		



		/*std::set<std::string>::const_iterator iter = g_auth_info.find(g_cfg.get_string(ECI_AuthPass));
		if (iter != g_auth_info.end())
		{
			return true;
		}
		*/
		WARNING_EX_LOG("auth_pass (%s) failed  !!!", g_cfg.get_string(ECI_AuthPass).c_str());
		return false;
	}

}



