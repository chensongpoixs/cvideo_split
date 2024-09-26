/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/


#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include <mutex>
#include "eXosip2/eXosip.h"
#include "cglobal_vms_port_mgr.h"
#include "cvms_device.h"
#include "cvms_device_util.h"
#include "clog.h"
namespace chen {




	void cvms_device::_handler_vms_registeration_success(const std::shared_ptr<eXosip_event_t>& event)
	{
		NORMAL_EX_LOG("");
		m_is_register = true;
	}
	void cvms_device::_handler_vms_registeration_failure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");

		if (!event->response)
		{
			WARNING_EX_LOG("vms register 401 has no response !!!");
			return;
		}
		// 注册流程
		if (401 == event->response->status_code)
		{
			osip_www_authenticate_t* www_authenticate_header ;

			osip_message_get_www_authenticate(event->response, 0, &www_authenticate_header);

			if (eXosip_add_authentication_info(m_vms_context_ptr, m_vms_device_id.c_str()/*m_vms_device_id.c_str()*/, m_username.c_str(), m_password.c_str(),
				"MD5", www_authenticate_header->realm)) 
			{
				WARNING_EX_LOG("vms register add auth failed");
				//return;
			}
			else
			{
				NORMAL_EX_LOG("add authentication info !!!");
			}
		}
	}
	void cvms_device::_handler_vms_call_invite(const std::shared_ptr<eXosip_event_t>& event)
	{
		/*
		流程描述如下： 
			1：终端设备向视频管理设备发送 Invite 请求，消息头域中携带 Subject 字段，表明点播的视频
			源 ID、分辨率、视频流接收者 ID、接收端视频流序列号等参数。携带 SDP 消息体，s 字段为
			“PlayMulticast”代表实时组播播放，增加 y 字段描述 SSRC 值，f 字段描述视频参数， m 字段描述
			了接收实时视频流的视频格式等内容； 
			2：如果摄像机离线，视频管理设备发送响应 495，如果摄像机正常，则回复 200 OK，携带 SDP
			信息，包含摄像机的组播地址与组播端口，视频格式、SSRC 字段等内容； 
			3：终端设备向视频管理设备回复 ACK 确认消息，完成组播实时播放的协商过程，终端设备可以接
			受组播流进行实时播放； 
			4：终端设备向视频管理设备发送 BYE 消息，断开消息 1、2、3 建立的会话； 
			5：终端设备接收到 BYE 消息后回复 200 OK； 
			说明： 
			a) 摄像机开启组播模式后，主动向组播地址发送媒体流，不需 RTSP 叫流。摄像机发送的媒体流
			格式要求为 PS 或 TS，均不带 RTP 包头。 
			b) 支持多通道输入的编码器，各通道单独配置组播地址和组播端口。


			play                                          device 
			 
			              1. invite(sdp) --->             _handler_vms_call_invite
						  2. 200 OK(sdp) <---                
						  3. ACK         --->             _handler_vms_call_ack

						<=====  主播媒体流 
						  4. BYE         ---> 
						  5. 200 OK      <---
			消息范例： 
 
				1 INVITE sip:视频流发送者设备编码@目的域名或 IP 地址端口 SIP/2.0 
				To: sip:视频流发送者设备编码@目的域名 
				Content-Length: 消息实体的字节长度 
				Contact: <sip:视频流接收者设备编码@源 IP 地址端口> 
				CSeq: 1 INVITE 
				Call-ID: wlss-f7c53b46-eea27828118c3b50449185980f4bfdf0@172.20.16.4 
				Via: SIP/2.0/UDP 源域名或 IP 地址 
				From: <sip:视频流接收者设备编码@源域名>;tag=e3719a0b 
				Subject: 视频流发送者设备编码:发送端视频流序列号,视频流接收者设备编码:接收端视频流序
				列号 
				Content-Type: APPLICATION/SDP 
				Max-Forwards: 70 
 
				v=0 
				o=0210011320001 0 0 IN IP4 172.20.16.3 
				s= PlayMulticast 
				c=IN IP4 172.20.16.3 
				t=0 0 
				m=video 6000 UDP 127 98 126 
				a=recvonly 
				a=udp:127 TS/90000 
				a=udp:98 H264/90000 
				a=udp:126 AAC/90000 
 
				2 SIP/2.0 200 OK 
				To: <sip:视频流发送者设备编码@目的域名>;tag=949c43d7 
				Contact: <sip:视频流发送者设备编码@目的 IP 地址端口> 
				Content-Length: 消息实体的字节长度 
				CSeq: 1 INVITE 
				Call-ID: wlss-f7c53b46-eea27828118c3b50449185980f4bfdf0@172.20.16.4 

		*/
		
		NORMAL_EX_LOG("");
		sdp_message_t* sdp_msg = eXosip_get_remote_sdp(m_vms_context_ptr, event->did);
		if (!sdp_msg) {
			WARNING_EX_LOG("eXosip_get_remote_sdp failed");
			return;
		}

		sdp_connection_t* connection = eXosip_get_video_connection(sdp_msg);
		if (!connection) {
			WARNING_EX_LOG("eXosip_get_video_connection failed");
			return;
		}


		//rtp_ip = connection->c_addr;

		auto video_sdp = eXosip_get_video_media(sdp_msg);
		if (!video_sdp) {
			WARNING_EX_LOG("eXosip_get_video_media failed");
			return;
		}

		//rtp_port = atoi(video_sdp->m_port);

		//NORMAL_EX_LOG("rtp server: {%s}:{%u}", rtp_ip.c_str(), rtp_port);

		//rtp_protocol = video_sdp->m_proto;

		//NORMAL_EX_LOG("rtp protocol: {%s}", rtp_protocol.c_str());

		osip_body_t* sdp_body = NULL;
		osip_message_get_body(event->request, 0, &sdp_body);
		if (nullptr == sdp_body)
		{
			WARNING_EX_LOG("osip_message_get_body failed");
			return;
		}
		// o=0210011320001 0 0 IN IP4 172.20.16.3 
		std::string body = sdp_body->body;
		// o=
		auto y_sdp_first_index = body.find("o=");
		auto y_sdp = body.substr(y_sdp_first_index);
		auto y_sdp_last_index = y_sdp.find("\r\n");
		std::string cmd_line = y_sdp.substr(2, y_sdp_last_index - 1);
		NORMAL_EX_LOG("ssrc: {%s}", cmd_line.c_str());
		auto o_channel_name = cmd_line.find(" ");
		std::string channel_name_ = cmd_line.substr(0, o_channel_name - 1);

		// c=
		auto c_sdp_first_index = body.find("c=");
		auto c_sdp = body.substr(c_sdp_first_index);
		auto c_sdp_last_index = y_sdp.find("\r\n");
		std::string ccmd_line = y_sdp.substr(2, c_sdp_last_index - 1);
		 
		std::stringstream ss;
		ss << "v=0\r\n";
		// o=0210011320001 0 0 IN IP4 172.18.16.1
		ss << "o=" << channel_name_ << " 0 0 IN IP4 " + m_local_port << "\r\n";
		ss << "s=##ms20090428 log-restart-callid-ssrc-reinvite\r\n";
		ss << "c=" << ccmd_line<< "\r\n";
		ss << "t=0 0\r\n";
		//if (rtp_protocol == "TCP/RTP/AVP") {
		//	ss << "m=video " << local_port << " TCP/RTP/AVP 96\r\n";
		//}
		//else {
			//ss << "m=video " << local_port << " RTP/AVP 96\r\n";
		//}
		ss << "m=video 6000 RTP/AVP 96 98";
		ss << "a=sendonly\r\n";

		ss << "a=rtpmap:96 PS/90000\r\n";
		ss << "a=rtpmap:98 H264/90000\r\n";
		ss << "a=rtpmap:97 MPEG4/90000\r\n";
		//ss << "y=" << ssrc << "\r\n";
		std::string sdp_output_str = ss.str();

		size_t size = sdp_output_str.size();

		osip_message_t* message = event->request;
		int status = eXosip_call_build_answer(m_vms_context_ptr, event->tid, 200, &message);

		if (status != 0) {
			WARNING_EX_LOG("call invite build answer failed");
			return;
		}

		osip_message_set_content_type(message, "APPLICATION/SDP");
		osip_message_set_body(message, sdp_output_str.c_str(), sdp_output_str.size());

		eXosip_call_send_answer(m_vms_context_ptr, event->tid, 200, message);

		 NORMAL_EX_LOG("reply call invite: \n{%s}", sdp_output_str.c_str());
	}
	void cvms_device::_handler_vms_call_reinvite(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}


	void cvms_device::_handler_vms_call_noanswer(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_proceeding(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_ringing(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_answered(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_redirected(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_requestfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_serverfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_globalfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_ack(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
		 

	void cvms_device::_handler_vms_call_closed(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_cancelled(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_call_message_new(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_proceeding(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_answered(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_redirected(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_requestfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_serverfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_call_message_globalfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}


	/*void cvms_device::_handler_vms_call_closed(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}*/
	void cvms_device::_handler_vms_call_released(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}


	void cvms_device::_handler_vms_message_new(const std::shared_ptr<eXosip_event_t>& event)
	{
		NORMAL_EX_LOG("");
		if (MSG_IS_MESSAGE(event->request)) 
		{
			osip_body_t* body = nullptr;
			osip_message_get_body(event->request, 0, &body);
			if (body != nullptr) 
			{
				NORMAL_EX_LOG("new message request: \n{%s}", body->body);
			}

			vms_send_response_ok(event);


			std::tuple<std::string, std::string> cmd_sn = cvms_device_util::get_vms_cmd(body->body);

			std::string cmd = std::get<0>(cmd_sn);
			std::string sn = std::get<1>(cmd_sn);
			NORMAL_EX_LOG("[cmd = %s][sn = %s]", cmd.c_str(), sn.c_str());
			if ("Catalog" == cmd) 
			{
				_process_catalog_query(sn);
				//this->process_catalog_query(sn);
			}
			else if ("DeviceStatus" == cmd) 
			{
				_process_devicestatus_query(sn);
				//this->process_devicestatus_query(sn);
			}
			else if ("DeviceInfo" == cmd) 
			{
				_process_deviceinfo_query(sn);
				//this->process_deviceinfo_query(sn);
			}
			else if ("DeviceControl" == cmd) 
			{
				_process_devicecontrol_query(sn);
				//this->process_devicecontrol_query(sn);
			}
			else 
			{
				WARNING_EX_LOG("unhandled cmd: {%s} == > (body->body = %s", cmd.c_str(), body->body);
			}
			/*auto cmd_sn = this->get_cmd(body->body);
			std::string cmd = std::get<0>(cmd_sn);
			std::string sn = std::get<1>(cmd_sn);
			NORMAL_EX_LOG("got new cmd: {%s}", cmd.c_str());
			if ("Catalog" == cmd) {
				this->process_catalog_query(sn);
			}
			else if ("DeviceStatus" == cmd) {
				this->process_devicestatus_query(sn);
			}
			else if ("DeviceInfo" == cmd) {
				this->process_deviceinfo_query(sn);
			}
			else if ("DeviceControl" == cmd) {
				this->process_devicecontrol_query(sn);
			}
			else {
				WARNING_EX_LOG("unhandled cmd: {%s} == > (body->body = %s", cmd.c_str(), body->body);
			}*/
		}
		else if (MSG_IS_BYE(event->request))
		{
			WARNING_EX_LOG("got BYE message");
			//this->send_response_ok(evt);
			vms_send_response_ok(event);
		}
		else
		{
			WARNING_EX_LOG(" vms message request   method  = %s", event->request->sip_method);
		}
	}
	void cvms_device::_handler_vms_message_proceeding(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_message_answered(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_message_redirected(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");

	}
	void cvms_device::_handler_vms_message_requestfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_message_serverfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_message_globalfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_subscription_noanswer(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_proceeding(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_answered(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_redirected(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_requestfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_serverfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_globalfailure(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}
	void cvms_device::_handler_vms_subscription_notify(const std::shared_ptr<eXosip_event_t>& event)
	{
		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_in_subscription_new(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}


	void cvms_device::_handler_vms_notification_noanswer(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}


	void cvms_device::_handler_vms_notification_proceeding(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_notification_answered(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_notification_redirected(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_notification_requestfailure(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_notification_serverfailure(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	void cvms_device::_handler_vms_notification_globalfailure(const std::shared_ptr<eXosip_event_t>& event)
	{

		WARNING_EX_LOG("");
	}

	

}

