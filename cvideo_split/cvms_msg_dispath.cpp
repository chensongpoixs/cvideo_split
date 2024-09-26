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


#include "cvms_msg_dispath.h"



namespace chen {

	cvms_msg_dispatch g_vms_msg_dispatch;

	cvms_msg_dispatch::cvms_msg_dispatch()
	{
	}

	cvms_msg_dispatch::~cvms_msg_dispatch()
	{
	}

	bool cvms_msg_dispatch::init()
	{
		_register_msg_handler(EXOSIP_REGISTRATION_SUCCESS, "vms_registeration_SUCCESS", &cvms_device::_handler_vms_registeration_success);
		_register_msg_handler(EXOSIP_REGISTRATION_FAILURE, "vms_registeration_failure", &cvms_device::_handler_vms_registeration_failure);

		// call 
		_register_msg_handler(EXOSIP_CALL_INVITE, "vms_call_invite", &cvms_device::_handler_vms_call_invite);
		_register_msg_handler(EXOSIP_CALL_REINVITE, "vms_call_reinvite", &cvms_device::_handler_vms_call_reinvite);
		_register_msg_handler(EXOSIP_CALL_NOANSWER, "vms_Call_noanswer", &cvms_device::_handler_vms_call_noanswer);
		_register_msg_handler(EXOSIP_CALL_PROCEEDING, "vms_call_proceeding", &cvms_device::_handler_vms_call_proceeding);
		_register_msg_handler(EXOSIP_CALL_RINGING, "VMS_CALL_RINGING", &cvms_device::_handler_vms_call_ringing);
		_register_msg_handler(EXOSIP_CALL_ANSWERED, "VMS_CALL_ANSWERED", &cvms_device::_handler_vms_call_answered);
		_register_msg_handler(EXOSIP_CALL_REDIRECTED, "VMS_CALL_REDIRECTED", &cvms_device::_handler_vms_call_redirected);
		_register_msg_handler(EXOSIP_CALL_REQUESTFAILURE, "VMS_CALL_REQUESTFAILURE", &cvms_device::_handler_vms_call_requestfailure);
		_register_msg_handler(EXOSIP_CALL_SERVERFAILURE, "VMS_CALL_SERVERFAILURE", &cvms_device::_handler_vms_call_serverfailure);
		_register_msg_handler(EXOSIP_CALL_GLOBALFAILURE, "VMS_CALL_GLOBALFAILURE", &cvms_device::_handler_vms_call_globalfailure);
		_register_msg_handler(EXOSIP_CALL_ACK, "VMS_CALL_ACK", &cvms_device::_handler_vms_call_ack); 
		_register_msg_handler(EXOSIP_CALL_CANCELLED, "VMS_CALL_CANCELLED", &cvms_device::_handler_vms_call_cancelled);

		// call message 
		_register_msg_handler(EXOSIP_CALL_MESSAGE_NEW, "VMS_CALL_MESSAGE_NEW", &cvms_device::_handler_vms_call_message_new);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_PROCEEDING, "VMS_CALL_MESSAGE_PROCEEDING", &cvms_device::_handler_vms_call_message_proceeding);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_ANSWERED, "VMS_CALL_MESSAGE_ANSWERED", &cvms_device::_handler_vms_call_message_answered);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_REDIRECTED, "VMS_CALL_MESSAGE_REDIRECTED", &cvms_device::_handler_vms_call_message_redirected);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_REQUESTFAILURE, "VMS_CALL_MESSAGE_REQUESTFAILURE", &cvms_device::_handler_vms_call_message_requestfailure);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_SERVERFAILURE, "VMS_CALL_MESSAGE_SERVERFAILURE", &cvms_device::_handler_vms_call_message_serverfailure);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_GLOBALFAILURE, "VMS_CALL_MESSAGE_GLOBALFAILURE", &cvms_device::_handler_vms_call_message_globalfailure);
		_register_msg_handler(EXOSIP_CALL_CLOSED, "VMS_CALL_CLOSED", &cvms_device::_handler_vms_call_closed);

		_register_msg_handler(EXOSIP_CALL_RELEASED, "VMS_CALL_RELEASED", &cvms_device::_handler_vms_call_released);

		// message
		_register_msg_handler(EXOSIP_MESSAGE_NEW, "VMS_MESSAGE_NEW", &cvms_device::_handler_vms_message_new);
		_register_msg_handler(EXOSIP_MESSAGE_PROCEEDING, "VMS_MESSAGE_PROCEEDING", &cvms_device::_handler_vms_message_proceeding);
		_register_msg_handler(EXOSIP_MESSAGE_ANSWERED, "VMS_MESSAGE_ANSWERED", &cvms_device::_handler_vms_message_answered);
		_register_msg_handler(EXOSIP_MESSAGE_REDIRECTED, "VMS_MESSAGE_REDIRECTED", &cvms_device::_handler_vms_message_redirected);
		_register_msg_handler(EXOSIP_MESSAGE_REQUESTFAILURE, "VMS_MESSAGE_REQUESTFAILURE", &cvms_device::_handler_vms_message_requestfailure);
		_register_msg_handler(EXOSIP_MESSAGE_SERVERFAILURE, "VMS_MESSAGE_SERVERFAILURE", &cvms_device::_handler_vms_message_serverfailure);
		_register_msg_handler(EXOSIP_MESSAGE_GLOBALFAILURE, "VMS_MESSAGE_GLOBALFAILURE", &cvms_device::_handler_vms_message_globalfailure);



		// subscription 
		_register_msg_handler(EXOSIP_SUBSCRIPTION_NOANSWER, "VMS_SUBSCRIPTION_NOANSWER", &cvms_device::_handler_vms_subscription_noanswer);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_PROCEEDING, "VMS_SUBSCRIPTION_PROCEEDING", &cvms_device::_handler_vms_subscription_proceeding);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_ANSWERED, "VMS_SUBSCRIPTION_ANSWERED", &cvms_device::_handler_vms_subscription_answered);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_REDIRECTED, "VMS_SUBSCRIPTION_REDIRECTED", &cvms_device::_handler_vms_subscription_redirected);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_REQUESTFAILURE, "VMS_SUBSCRIPTION_REQUESTFAILURE", &cvms_device::_handler_vms_subscription_requestfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_SERVERFAILURE, "VMS_SUBSCRIPTION_SERVERFAILURE", &cvms_device::_handler_vms_subscription_serverfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_GLOBALFAILURE, "VMS_SUBSCRIPTION_GLOBALFAILURE", &cvms_device::_handler_vms_subscription_globalfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_NOTIFY, "VMS_SUBSCRIPTION_NOTIFY", &cvms_device::_handler_vms_subscription_notify);

		_register_msg_handler(EXOSIP_IN_SUBSCRIPTION_NEW, "VMS_IN_SUBSCRIPTION_NEW", &cvms_device::_handler_vms_in_subscription_new);


		// nofification 
		_register_msg_handler(EXOSIP_NOTIFICATION_NOANSWER, "VMS_NOTIFICATION_NOANSWER", &cvms_device::_handler_vms_notification_noanswer);
		_register_msg_handler(EXOSIP_NOTIFICATION_PROCEEDING, "VMS_NOTIFICATION_PROCEEDING", &cvms_device::_handler_vms_notification_proceeding);
		_register_msg_handler(EXOSIP_NOTIFICATION_ANSWERED, "VMS_NOTIFICATION_ANSWERED", &cvms_device::_handler_vms_notification_answered);
		_register_msg_handler(EXOSIP_NOTIFICATION_REDIRECTED, "VMS_NOTIFICATION_REDIRECTED", &cvms_device::_handler_vms_notification_redirected);
		_register_msg_handler(EXOSIP_NOTIFICATION_REQUESTFAILURE, "VMS_NOTIFICATION_REQUESTFAILURE", &cvms_device::_handler_vms_notification_requestfailure);

		_register_msg_handler(EXOSIP_NOTIFICATION_SERVERFAILURE, "VMS_NOTIFICATION_SERVERFAILURE", &cvms_device::_handler_vms_notification_serverfailure);
		_register_msg_handler(EXOSIP_NOTIFICATION_GLOBALFAILURE, "VMS_NOTIFICATION_GLOBALFAILURE", &cvms_device::_handler_vms_notification_globalfailure);







		return true;
	}

	void cvms_msg_dispatch::udpate(uint32 uDataTime)
	{

	}

	void cvms_msg_dispatch::destroy()
	{
	}

	cvms_msg_handler* cvms_msg_dispatch::get_msg_handler(uint16 msg_id)
	{
		if (/*static_cast<int> (msg_id) <= EXOSIP_REGISTRATION_SUCCESS || */ static_cast<int> (msg_id) >= EXOSIP_EVENT_COUNT)
		{
			return NULL;
		}

		return &(m_vms_msg_handler[msg_id]);
	}

	void cvms_msg_dispatch::_register_msg_handler(uint16 msg_id, const std::string& msg_name, handler_type handler)
	{
		if (/*static_cast<int> (msg_id) <= Msg_Client_Max ||*/ m_vms_msg_handler[msg_id].handler && msg_id >= EXOSIP_EVENT_COUNT)
		{
			ERROR_LOG("[%s] register msg error, msg_id = %u, msg_name = %s", __FUNCTION__, msg_id, msg_name.c_str());
			return;
		}

		m_vms_msg_handler[msg_id].msg_name = msg_name;//   数据统计
		m_vms_msg_handler[msg_id].handle_count = 0;
		m_vms_msg_handler[msg_id].handler = handler;
	}

}