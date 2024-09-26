/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr
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
*********************************************************************/

#ifndef  _C_VMS_DEVICE_H_
#define  _C_VMS_DEVICE_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include <mutex>
#include "eXosip2/eXosip.h"
#include "cglobal_vms_port_mgr.h"

namespace chen {



	class cvms_device
	{
	public:
		explicit cvms_device() 
		:m_vms_context_ptr(NULL)
		, m_stoped(false)
		, m_local_ip(128, '0')
		, m_local_port(0)
		, m_vms_server_device_id("")
		, m_vms_server_ip("")
		, m_vms_server_port(0)
		, m_vms_device_id("")
	 	//, m_vms_channel_id("")
		, m_username("")
		, m_password("")
		, m_manufacture("")
		, m_is_register(false)
		, m_from_sip("")
		, m_to_sip("")
		, m_sn(0)
		, m_heartbeat(::time(NULL))
		, m_channel_heartbeat(::time(NULL))
		{}
		virtual ~cvms_device();
	public:
		bool init();
		void update(uint32 uDataTime);
		void destroy();


		void stop();
	public:
		void  vms_send_response_ok(const std::shared_ptr<eXosip_event_t>& event);



		void vms_send_all_channel_info();
	private: 


		int32 get_sn();  

		void _process_catalog_query(const std::string& sn);
		void _process_devicestatus_query(const std::string & sn);
		void _process_deviceinfo_query(const std::string & sn);
		void _process_devicecontrol_query(const std::string & sn);
	private:

		void _vms_send_request(osip_message_t* request);
		void _vms_send_response(const std::shared_ptr<eXosip_event_t>& event, osip_message_t* msg);


		

		osip_message_t* _create_msg();
	public:

		void _handler_vms_registeration_success(const std::shared_ptr<eXosip_event_t> & event);
		void _handler_vms_registeration_failure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_invite(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_reinvite(const std::shared_ptr<eXosip_event_t>& event);


		void _handler_vms_call_noanswer(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_proceeding(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_ringing(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_answered(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_redirected(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_requestfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_serverfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_globalfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_ack(const std::shared_ptr<eXosip_event_t>& event);
		

		void _handler_vms_call_closed(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_cancelled(const std::shared_ptr<eXosip_event_t>& event);

		void _handler_vms_call_message_new(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_proceeding(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_answered(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_redirected(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_requestfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_serverfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_message_globalfailure(const std::shared_ptr<eXosip_event_t>& event);


		//void _handler_vms_call_closed(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_call_released(const std::shared_ptr<eXosip_event_t>& event);


		void _handler_vms_message_new(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_proceeding(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_answered(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_redirected(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_requestfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_serverfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_message_globalfailure(const std::shared_ptr<eXosip_event_t>& event);

		void _handler_vms_subscription_noanswer(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_proceeding(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_answered(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_redirected(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_requestfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_serverfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_globalfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_subscription_notify(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_in_subscription_new(const std::shared_ptr<eXosip_event_t>& event);

		void _handler_vms_notification_noanswer(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_proceeding(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_answered(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_redirected(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_requestfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_serverfailure(const std::shared_ptr<eXosip_event_t>& event);
		void _handler_vms_notification_globalfailure(const std::shared_ptr<eXosip_event_t>& event);
	
	private:
		void _pthread_work();
	private:

		eXosip_t*			m_vms_context_ptr;
		bool				m_stoped;
		std::string			m_local_ip;
		uint32				m_local_port;

		// vms 
		std::string			m_vms_server_device_id;
		std::string			m_vms_server_ip;
		uint32				m_vms_server_port;
		std::string			m_vms_device_id;
		//std::string			m_vms_channel_id;
		std::string			m_username;
		std::string			m_password;
		std::string			m_manufacture;
		bool				m_is_register;

		// 
		std::string			m_from_sip;
		std::string			m_to_sip;


		std::thread			m_thread;
		int32				m_sn; // 命令序列号
		time_t				m_heartbeat;
		time_t				m_channel_heartbeat;
	};


	extern  cvms_device g_vms_device_mgr;
}


#endif // _C_VMS_DEVICE_H_