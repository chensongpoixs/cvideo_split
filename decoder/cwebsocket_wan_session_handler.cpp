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

 
#include "cnoncopytable.h"
#include "cnet_type.h"
#include <list>
#include <json/json.h>
#include "cwebsocket_wan_session.h"
#include "clog.h"
#include "cwebsocket_wan_server.h"
namespace chen {
	
	bool    cwebsocket_wan_session::handler_play_url(Json::Value& value)
	{
		Json::Value reply;
		if (!value.isMember("url") || !value["url"].isString())
		{
			WARNING_EX_LOG("[remote_ip = %s][remote_port = %u]", m_remote_ip.c_str(), m_remote_port);
			return false;
		}
		if (m_decoder_ptr)
		{
			WARNING_EX_LOG("decoder alloc ---> != NULL  ^_^");
			return false;
		}
		std::string url  = value["url"].asCString();
		m_decoder_ptr =   cdecoder::construct();
		if (!m_decoder_ptr)
		{
			WARNING_EX_LOG("[url = %s]alloc decoder  failed !!!", url.c_str());
			return false;
		}
		if (!m_decoder_ptr->init(m_session_id, url.c_str()))
		{
			m_decoder_ptr->destroy();
			delete m_decoder_ptr;
			m_decoder_ptr = NULL;
			uint32 codec_id[8];
			codec_id[0] = 28;
			g_websocket_wan_server.send_msg(m_session_id, 323, &codec_id, sizeof(codec_id));
			uint32 buffer[1024];
			g_websocket_wan_server.send_msg(m_session_id, 323, &buffer, 1024);
			WARNING_EX_LOG("[url = %s]  decoder init  failed !!!", url.c_str());
			return false;
		}
		// 启动编码器读取数据发送数据
		//if (!value.isMember("data") || !value["data"].isObject())
		//{
		//	WARNING_EX_LOG("[session_id = %llu]not find cmd type, [value = %s] !!! ", m_session_id, value.asCString());
		//	send_msg(S2C_Login, EShareProtoData, reply);
		//	return false;
		//}
		//if (!value["data"].isMember("user_name") || !value["data"]["user_name"].isString())
		//{
		//	WARNING_EX_LOG("[session_id = %llu]not find user_name      !!! ", m_session_id);
		//	send_msg(S2C_Login, EShareProtoUserName, reply);
		//	return false;
		//}
		//if (!value["data"].isMember("room_name") || !value["data"]["room_name"].isString())
		//{
		//	WARNING_EX_LOG("[session_id = %llu]not find room_name    !!! ", m_session_id);
		//	send_msg(S2C_Login, EShareProtoRoomName, reply);
		//	return false;
		//}
 
		return true;
	}
	
}

 
