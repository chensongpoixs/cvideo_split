/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#include "cvms_device.h"
#include "clog.h"
#ifdef WIN32
/* #include <syslog.h> */
#include <winsock2.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#endif
#include <string>
#include <sstream>
#include <thread>
#include <tuple> 
#include "clog.h"
#include <cstdio>
#include <cstdlib>
#include "cvms_msg_dispath.h"
#include <cstring>
#include "ccfg.h"
namespace chen {

	cvms_device g_vms_device_mgr;
	/*cvms_device::cvms_device()
	{
	}*/
	cvms_device::~cvms_device()
	{
	}
	bool cvms_device::init()
	{

		m_vms_context_ptr = ::eXosip_malloc();
		if (!m_vms_context_ptr)
		{
			WARNING_EX_LOG("sip alloc failed !!!");
			return false;
		}
		if (OSIP_SUCCESS != ::eXosip_init(m_vms_context_ptr))
		{
			WARNING_EX_LOG("sip init failed !!!");
			return false;
		}


		// TODO@chensong 20240924  bind //分配端口
		// 
		m_local_port = g_global_vms_port_mgr.get_local_vms_port();

		if (OSIP_SUCCESS != ::eXosip_listen_addr(m_vms_context_ptr, IPPROTO_UDP, nullptr, m_local_port, AF_INET, 0))
		{
			WARNING_EX_LOG("vms bind [port = %u] failed !!!", m_local_port);
			return false;
		}


		//获取本地ip 
		if (OSIP_SUCCESS != eXosip_guess_localip(m_vms_context_ptr, AF_INET, (char*)(m_local_ip.c_str()), m_local_ip.length()))
		{
			WARNING_EX_LOG("vms get local ip failed !!!");
			return false;
		}
		NORMAL_EX_LOG("vms local ip  [%s]", m_local_ip.c_str());


		::eXosip_clear_authentication_info(m_vms_context_ptr);

		 std::ostringstream from_uri;
		std::ostringstream contact;
		std::ostringstream proxy_uri;

		// 通道
		m_vms_server_ip = g_cfg.get_string(ECI_VmsServerIp);
		m_vms_server_port = g_cfg.get_uint32(ECI_VmsServerPort);
		
		m_vms_server_device_id = g_cfg.get_string(ECI_VmsServerDeviceId);
		
		m_vms_device_id = g_cfg.get_string(ECI_VmsDeviceId);

		m_username = g_cfg.get_string(ECI_VmsUserName);
		m_password = g_cfg.get_string(ECI_VmsPassWord);

		//
	//	m_vms_channel_id = "31011500991320000342";
		from_uri << "sip:" << m_vms_device_id << "@" << m_local_ip << ":" << m_local_port;
		contact << "sip:" << m_vms_device_id /*m_vms_channel_id */<< "@" << m_local_ip << ":" << m_local_port;
		proxy_uri << "sip:" << m_vms_server_device_id << "@" << m_vms_server_ip << ":" << m_vms_server_port;
	//	NORMAL_EX_LOG("contact = %s", contact.c)

		m_from_sip = from_uri.str();
		m_to_sip = proxy_uri.str();
		osip_message_t* register_message = nullptr;
		int32 register_id = eXosip_register_build_initial_register(m_vms_context_ptr, m_from_sip.c_str(),
			m_to_sip.c_str(), contact.str().c_str(), 3600, &register_message);
		if (nullptr == register_message) 
		{
			ERROR_EX_LOG("eXosip_register_build_initial_register failed");
			return false ;
		}
		//printf("[%s][%d]\n", __FUNCTION__, __LINE__);

		eXosip_lock(m_vms_context_ptr);
		uint32 result_register = eXosip_register_send_register(m_vms_context_ptr, register_id, register_message);
		eXosip_unlock(m_vms_context_ptr);
		if (OSIP_SUCCESS != result_register)
		{
			WARNING_EX_LOG("vms register send failed !!!");
			return false;
		}
		m_stoped = false;

		m_thread = std::thread(&cvms_device::_pthread_work, this);
		
		//printf("[%s][%d]\n", __FUNCTION__, __LINE__);

		//std::thread heartbeat_task_thread(&Device::heartbeat_task, this);
		//printf("[%s][%d]\n", __FUNCTION__, __LINE__);
		//heartbeat_task_thread.detach();


		return true;
	}
	void cvms_device::update(uint32 uDataTime)
	{


		// 5s 秒发送一次心
		if (::time(NULL) - m_heartbeat > g_cfg.get_uint32(ECI_VmsHeartBeat))
		{
			m_heartbeat = ::time(NULL);
			if (m_is_register)
			{
				std::stringstream ss;
				ss << "<?xml version=\"1.0\"?>\r\n";
				ss << "<Notify>\r\n";
				ss << "<CmdType>Keepalive</CmdType>\r\n";
				ss << "<SN>" << get_sn() << "</SN>\r\n";
				ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
				ss << "<Status>OK</Status>\r\n";
				ss << "</Notify>\r\n";

				osip_message_t* request = _create_msg();
				if (request != NULL) {
					osip_message_set_content_type(request, "Application/MANSCDP+xml");
					osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
					_vms_send_request(request);
					NORMAL_EX_LOG("sent heartbeat");
				}
			}
		}
		vms_send_all_channel_info();
		


	}
	void cvms_device::destroy()
	{

		m_stoped = true;
		if (m_vms_context_ptr)
		{
			::eXosip_quit(m_vms_context_ptr);
			m_vms_context_ptr = NULL;
		}
	}
	void cvms_device::stop()
	{
		m_stoped = true;
	}
	void cvms_device::vms_send_response_ok(const std::shared_ptr<eXosip_event_t>& event)
	{
		osip_message_t* msg = event->request;
		eXosip_message_build_answer(m_vms_context_ptr, event->tid, 200, &msg);
		_vms_send_response(event, msg);
	}

	void cvms_device::vms_send_all_channel_info()
	{
		if (::time(NULL) - m_channel_heartbeat < 30)
		{
			return;
		}
		m_channel_heartbeat = ::time(NULL);
		if (m_is_register)
		{
			// 

			std::stringstream ss;
			//ss << "<?xml version=\"1.0\"   encoding=\"GB2312\"  ?>\r\n";
			//ss << "<Response>\r\n";
			//ss << "<CmdType>Catalog</CmdType>\r\n";
			//ss << "<SN>" << get_sn() << "</SN>\r\n";
			//ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
			//ss << "<Status>OK</Status>\r\n";
			//ss << "</Notify>\r\n";

			 
			ss << "<?xml version=\"1.0\" encoding=\"GB2312\"  ?>\r\n";
			ss << "<Response>\r\n";
			ss << "<CmdType>Catalog</CmdType>\r\n";
			ss << "<SN>" << get_sn() << "</SN>\r\n";
			ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
			ss << "<SumNum>"<<8 /*中的通道数*/ <<"</SumNum>\r\n";
			ss << "<DeviceList Num = \"1\">\r\n";
			ss << "<Item>\r\n";
			ss << "<DeviceID>" << "1900242000001"/*每个通道*/ << "</DeviceID>\r\n";
			ss << "<Name>" << "channel_name" << "</Name>\r\n";
			ss << "<Manufacturer>" << g_cfg.get_string(ECI_VmsDeviceManufacturer) << "</Manufacturer>\r\n";
			// 当为设备时，设备型号（必选）
			ss << "<Model>split_camera</Model>\r\n";
			// 当为设备时，设备归属（必选）
			ss << "<Owner>syz</Owner>\r\n";
			// 行政区域（必选）
			ss << "<CivilCode>1230011180101</CivilCode>\r\n";
			// 当为设备时，是否有子设备（必选） 1 有， 0 没有
			ss << "<Parental>0</Parental>\r\n";
			// 父设备/区域/系统 ID（必选）
			ss << "<ParentID>1200201180101</ParentID>\r\n";
			// 设备/区域/系统 IP 地址（可选） 
			ss << "<Address></Address>\r\n";
			// 注册方式（必选）缺省为 1； 1：符合 IETF RFC 3261 标准的认证注册模式； 2：基于口令的
			// 双向认证注册模式； 3：基于数字证书的双向认证注册模式 -
			ss << "<RegisterWay>1</RegisterWay>\r\n";
			// 保密属性（必选）缺省为 0； 0：不涉密， 1：涉密
				ss << "<Secrecy>0</Secrecy>\r\n";
			// 设备状态（必选） 
			ss << "<Status>1</Status>\r\n";
			// 设备/区域/系统 IP 地址（可选）
			ss << "<IPAddress>"<< m_local_ip <<"</IPAddress>\r\n";
			// 设备/区域/系统端口（可选）
			ss << "<Port>"<< m_local_port <<"</Port>\r\n";
			ss << "<Info></Info>\r\n";
			ss << "</Item>\r\n";
			ss << "</DeviceList>\r\n";
			ss << "</Response>\r\n";
			
			 
		}

	}

	int32 cvms_device::get_sn()
	{
		static const int32 SN_MAX = 99999999;
		if (m_sn >= SN_MAX)
		{
			m_sn = 0;
		}
		return ++m_sn;
	}

	void cvms_device::_process_catalog_query(const std::string& sn)
	{
		std::stringstream ss;
		ss << "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n";
		ss << "<Response>\r\n";
		ss << "<CmdType>Catalog</CmdType>\r\n";
		ss << "<SN>" << sn << "</SN>\r\n";
		ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
		ss << "<SumNum>" << 1 << "</SumNum>\r\n";
		ss << "<DeviceList Num=\"" << 1 << "\">\r\n";
		ss << "<Item>\r\n";
		ss << "<DeviceID>" << m_vms_device_id /*m_vms_channel_id*/ << "</DeviceID>\r\n";
		ss << "<Manufacturer>" << g_cfg.get_string(ECI_VmsDeviceManufacturer) << "</Manufacturer>\r\n";
		ss << "<Status>ON</Status>\r\n";
		ss << "<Channel Num=\"1\">\r\n";
		ss << "<Name>IPC</Name>\r\n";
		ss << "<ParentID>" << m_vms_server_device_id << "</ParentID>\r\n";
		ss << "</Item>\r\n";
		ss << "</DeviceList>\r\n";
		ss << "</Response>\r\n";
		NORMAL_EX_LOG("catalog response: \n{}", ss.str());
		auto request = _create_msg();
		if (request != NULL) 
		{
			osip_message_set_content_type(request, "Application/MANSCDP+xml");
			osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
			_vms_send_request(request);
		}
	}
	void cvms_device::_process_devicestatus_query(const std::string& sn)
	{
		std::stringstream ss;

		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char curtime[72] = { 0 };
		sprintf(curtime, "%d-%d-%dT%02d:%02d:%02d", (timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1),
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

		ss << "<?xml version=\"1.0\"?>\r\n";
		ss << "<Response>\r\n";
		ss << "<CmdType>DeviceStatus</CmdType>\r\n";
		ss << "<SN>" << get_sn() << "</SN>\r\n";
		ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
		ss << "<Result>OK</Result>\r\n";
		ss << "<Online>ONLINE</Online>\r\n";
		ss << "<Status>OK</Status>\r\n";
		ss << "<DeviceTime>" << curtime << "</DeviceTime>\r\n";
		ss << "<Alarmstatus Num=\"0\">\r\n";
		ss << "</Alarmstatus>\r\n";
		ss << "<Encode>ON</Encode>\r\n";
		ss << "<Record>OFF</Record>\r\n";
		ss << "</Response>\r\n";

		NORMAL_EX_LOG("devicestatus response: \n{%s}", ss.str().c_str());
		auto request = _create_msg();
		if (request != NULL)
		{
			osip_message_set_content_type(request, "Application/MANSCDP+xml");
			osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
			_vms_send_request(request);
		}
	}

	void cvms_device::_process_deviceinfo_query(const std::string& sn)
	{
		std::stringstream ss;

		ss << "<?xml version=\"1.0\"?>\r\n";
		ss << "<Response>\r\n";
		ss << "<CmdType>DeviceInfo</CmdType>\r\n";
		ss << "<SN>" << get_sn() << "</SN>\r\n";
		ss << "<DeviceID>" << m_vms_device_id << "</DeviceID>\r\n";
		ss << "<Result>OK</Result>\r\n";
		ss << "<DeviceType>simulate client</DeviceType>\r\n";
		ss << "<Manufacturer>" +g_cfg.get_string(ECI_VmsDeviceManufacturer)+ "</Manufacturer>\r\n";
		ss << "<Model>28181</Model>\r\n";
		ss << "<Firmware>fireware</Firmware>\r\n";
		ss << "<MaxCamera>1</MaxCamera>\r\n";
		ss << "<MaxAlarm>0</MaxAlarm>\r\n";
		ss << "</Response>\r\n";

		NORMAL_EX_LOG("deviceinfo response: \n{}", ss.str());
		auto request = _create_msg();
		if (request != NULL) 
		{
			osip_message_set_content_type(request, "Application/MANSCDP+xml");
			osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
			_vms_send_request(request);
		}
	}
	void cvms_device::_process_devicecontrol_query(const std::string& sn)
	{
	}

	osip_message_t* cvms_device::_create_msg()
	{
		osip_message_t* request = nullptr;
		int32 status = eXosip_message_build_request(m_vms_context_ptr, &request, "MESSAGE", m_to_sip.c_str(), m_from_sip.c_str(), nullptr);
		if (OSIP_SUCCESS != status) 
		{
			WARNING_EX_LOG("build request failed: {%u}", status);
		}

		return request;
	}
	void cvms_device::_vms_send_request(osip_message_t* request)
	{
		eXosip_lock(m_vms_context_ptr);
		eXosip_message_send_request(m_vms_context_ptr, request);
		eXosip_unlock(m_vms_context_ptr);
	}
	void cvms_device::_vms_send_response(const std::shared_ptr<eXosip_event_t>& event, osip_message_t* msg)
	{
		eXosip_lock(m_vms_context_ptr);
		eXosip_message_send_answer(m_vms_context_ptr, event->tid, 200, msg);
		eXosip_unlock(m_vms_context_ptr);
	}
	void cvms_device::_pthread_work()
	{

		while (!m_stoped)
		{
			std::shared_ptr<eXosip_event_t> vms_event_ptr = std::shared_ptr<eXosip_event_t>(
				eXosip_event_wait(m_vms_context_ptr, 0, 100), eXosip_event_free);
			eXosip_lock(m_vms_context_ptr);
			eXosip_automatic_action(m_vms_context_ptr);
			eXosip_unlock(m_vms_context_ptr);
			if (!vms_event_ptr)
			{
				WARNING_EX_LOG("sip vms event wait event == NULL failed !!! ");
				continue;
			}


			cvms_msg_handler * handler_ptr = g_vms_msg_dispatch.get_msg_handler(vms_event_ptr->type);
			if (!handler_ptr )
			{
				WARNING_EX_LOG("unhandler vms event [type  = %u] !!!", vms_event_ptr->type);
				continue;
			}
			if (!handler_ptr->handler)
			{
				WARNING_EX_LOG("handler vms event [type  = %u], call handler == NULL !!!", vms_event_ptr->type);
				continue;
			}
			(this->*(handler_ptr->handler)) (vms_event_ptr);
		}
	}
}