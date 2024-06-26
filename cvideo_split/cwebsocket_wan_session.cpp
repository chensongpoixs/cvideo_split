﻿/********************************************************************
created:	2019-05-07

author:		chensong

level:		网络层

purpose:	网络数据的收发
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
#include "cwebsocket_wan_session.h"
#include "cclient_msg_dispatch.h"
#include "clog.h"
#include "cwebsocket_wan_server.h"

namespace chen {
	cwebsocket_wan_session::~cwebsocket_wan_session()
	{
	}
	bool cwebsocket_wan_session::init(const char* ip, uint16_t port)
	{
		m_client_connect_type = EWebSocketConnectNone;
		m_remote_ip = ip;
		m_remote_port = port;
		return true;
	}
	void cwebsocket_wan_session::destroy()
	{
		m_client_connect_type = EWebSocketConnectNone;
		 
		m_session_id = 0;
	}
	void cwebsocket_wan_session::update(uint32 uDeltaTime)
	{
		/*if (m_decoder_ptr)
		{
			m_decoder_ptr->all_send_packet();
		}*/
	}
	void cwebsocket_wan_session::handler_msg(uint64_t session_id, const void* p, uint32 size)
	{
		// 1. 登录状态判断
		NORMAL_EX_LOG("[session_id = %u][p = %s]", session_id, p);
		if (!m_json_reader.parse((const char*)p, (const char*)p + size, m_json_response))
		{
			ERROR_EX_LOG("parse json failed !!! [session_id = %llu][json = %s]", session_id, p);
			return;
		}
		if (!m_json_response.isMember("msg_id"))
		{
			WARNING_EX_LOG("[session_id = %llu]not find cmd type !!!", session_id);
			return;
		}
		// TODO@chensong 20220812 管理的比较漏
		const uint32 msg_id = m_json_response["msg_id"].asUInt();
		cclient_msg_handler* client_msg_handler = g_client_msg_dispatch.get_msg_handler(msg_id);
		if (!client_msg_handler || !client_msg_handler->handler)
		{
			WARNING_EX_LOG("[session_id = %llu]not find msg_id= %u callback !!!", session_id, msg_id);
			return;
		}
		m_session_id = session_id;
		++client_msg_handler->handle_count;
		(this->*(client_msg_handler->handler))(m_json_response);
	}

	void cwebsocket_wan_session::close()
	{
		//WARNING_EX_LOG("");
		m_client_connect_type = EWebSocketConnectNone;
		 
	}

	bool cwebsocket_wan_session::is_used()
	{
		return m_client_connect_type > EWebSocketConnectNone;
	}
	void cwebsocket_wan_session::set_used()
	{
		m_client_connect_type = EWebSocketConnected;
	}
	void cwebsocket_wan_session::disconnect()
	{
		m_client_connect_type = EWebSocketConnectNone;

		/*if (m_decoder_ptr)
		{
			m_decoder_ptr->destroy();
			cdecoder::destroy(m_decoder_ptr);
			m_decoder_ptr = NULL;
		 }*/
		 
		NORMAL_EX_LOG("session_id = %u, [ip = %s][port = %u]", m_session_id, m_remote_ip.c_str(), m_remote_port);
		m_remote_ip = "";
		m_remote_port = 0;
	 
	}
	bool cwebsocket_wan_session::send_msg(uint16 msg_id, int32 result, Json::Value data)
	{
		Json::Value value;
		value["msg_id"] = msg_id;
		value["result"] = result;
		value["data"] = data;
		Json::StyledWriter swriter;
		std::string str = swriter.write(value);
		g_websocket_wan_server.send_msg(m_session_id, msg_id, str.c_str(), str.length());
		return true;
	}
}