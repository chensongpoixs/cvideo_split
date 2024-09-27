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


#ifndef _C_GLOBAL_VMS_SERVER_CONFIG_MGR_H_
#define _C_GLOBAL_VMS_SERVER_CONFIG_MGR_H_

#include <stdint.h>
//#include <GL/eglew.h>
#include <vector>
#include <string>
#include <thread>
#include "cffmpeg_util.h"
#include <list>
#include "cnet_type.h"
#include "cmpegts_encoder.h"
#include "cweb_http_struct.h"
#include "ccamera_info_mgr.h"
namespace chen {

	class cglobal_vms_server_config_mgr
	{
	public:
		explicit cglobal_vms_server_config_mgr()
			: m_vms_server_ip("")
			, m_vms_server_port(0)
			, m_vms_server_device_id("")
			, m_video_split_device_id("")
			, m_user_name("")
			, m_pass_word("")
		{}
		virtual ~cglobal_vms_server_config_mgr();
	
	public:

		bool init();
		void update(uint32 uDataTime);
		void destroy();



	public:
		cresult_vms_server_config handler_web_modify_vms_server_config(const cvms_server_config & server_config);
		cresult_vms_server_config handler_web_get_vms_server_config();

		uint32    handler_web_cmd_vms_server(uint32 cmd);
	private:
		void _write_vms_server_config();

		void _sync_save();
	private:
		void _load_vms_server_config();
		
		void _parse_json_data(const std::string & data);;
	private:

		EDataType						m_data_type;
		// server 
		std::string		m_vms_server_ip;
		uint32			m_vms_server_port;
		std::string		m_vms_server_device_id;

		// client 
		std::string		m_video_split_device_id;


		std::string		m_user_name;
		std::string		m_pass_word;


		
	};
	extern cglobal_vms_server_config_mgr g_global_vms_server_config_mgr;;
}


#endif // _C_GLOBAL_VMS_SERVER_CONFIG_MGR_H_
