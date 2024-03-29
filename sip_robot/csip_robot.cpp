/***********************************************************************************************
created: 		2023-11-18

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

#include "csip_robot.h"
#include "ccfg.h"
#include "ctime_api.h"
#include "ctime_elapse.h"
#include "clib_util.h"
#include "clog.h"
#include "device.h"
namespace chen {
	csip_robot g_sip_robot;
	bool csip_robot::init(const char* log_path, const char* config_file)
	{
		printf("Log init ...\n");
		if (!LOG::init(log_path, "sip_robot"))
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

		m_stoped = false;

		SYSTEM_LOG("sip Robot  server init ok");

		return true; 
	}
	bool csip_robot::Loop()
	{

		NORMAL_EX_LOG("<<<<<<<<<<<<<<<<<<<<<< device info <<<<<<<<<<<<<<<<<<<<");

		NORMAL_EX_LOG("[sip server id = %s]", g_cfg.get_string(ECI_SipServerId).c_str());
		NORMAL_EX_LOG("[sip server ip address = %s]", g_cfg.get_string(ECI_SipServerIp).c_str());
		NORMAL_EX_LOG("[sip server port = %u]", g_cfg.get_uint32(ECI_SipServerPort));
		NORMAL_EX_LOG("[sip device id = %s]", g_cfg.get_string(ECI_DeviceId).c_str());
		NORMAL_EX_LOG("[sip device channel id  = %s]", g_cfg.get_string(ECI_DeviceChannelId).c_str());
		NORMAL_EX_LOG("[sip device username  = %s]", g_cfg.get_string(ECI_DeviceUserName).c_str());
		NORMAL_EX_LOG("[sip device password = %s]", g_cfg.get_string(ECI_DevicePassWord).c_str());
		NORMAL_EX_LOG("[sip device manufacture = %s]", g_cfg.get_string(ECI_DeviceManuFacture).c_str());
		NORMAL_EX_LOG("[sip h264 file path = %s]", g_cfg.get_string(ECI_FilePath).c_str());

		NORMAL_EX_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");


		std::shared_ptr<Device> device_ptr = std::make_shared<Device>(
			g_cfg.get_string(ECI_SipServerId).c_str()
			, g_cfg.get_string(ECI_SipServerIp).c_str()
			, g_cfg.get_uint32(ECI_SipServerPort) 
			, g_cfg.get_string(ECI_DeviceId).c_str()
			, g_cfg.get_string(ECI_DeviceChannelId).c_str()
			, g_cfg.get_string(ECI_DeviceUserName).c_str()
			, g_cfg.get_string(ECI_DevicePassWord).c_str()
			, g_cfg.get_uint32(ECI_DevicePort)
			,  g_cfg.get_string(ECI_DeviceManuFacture).c_str()
			, g_cfg.get_string(ECI_FilePath).c_str()

			);
		if (device_ptr)
		{
			device_ptr->start();
		}
		return true;

		static const uint32 TICK_TIME = 100;
		//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È´ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½
		//NORMAL_EX_LOG("");
		ctime_elapse time_elapse;
		uint32 uDelta = 0;
		while (!m_stoped)
		{
			uDelta += time_elapse.get_elapse();
			//g_websocket_wan_server.update(uDelta);
			 

			uDelta = time_elapse.get_elapse();

			if (uDelta < TICK_TIME)
			{
				csleep(TICK_TIME - uDelta);
			}
		}

		SYSTEM_LOG("Leave main loop");

		return true;
		return true;
	}
	void csip_robot::Destroy()
	{
		g_cfg.destroy();
		SYSTEM_LOG("config destory OK !!!");
		LOG::destroy();
		printf(" sip Robot server Destroy End OK !!!\n");
	}
	void csip_robot::stop()
	{
		m_stoped = true;
	}
}