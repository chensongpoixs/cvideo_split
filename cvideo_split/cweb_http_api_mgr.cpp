

#include "cweb_http_api_mgr.h"
#include <iostream>
#include "ccfg.h"
#include <thread>
#include "cweb_http_api_proxy.h"
//#include <json.h>
#include <json/json.h>
#include <cinttypes>

//#include "cserver_msg_header.h"
#include "ccamera_mgr.h"

#include "crandom.h"

//#include "cguard_reply.h"
//#include "cmsg_base_id.h"
//#include "cserver_cfg.h"
#include "ccfg.h"
//#include "cglobal_render_app_mgr.h"
 
#include "chttp_code.h"
//#include "cglobal_logic_mgr.h"


namespace chen {

	#define	REGISTER_WEB_HANDLER(FUNC_NAME, METHOD, FUNC_CALLBACK)								 \
				if (!this->_register_web_handler(FUNC_NAME, METHOD, FUNC_CALLBACK)) {			 \
						ERROR_EX_LOG("register_web_handler[method = %s] [func_name = %s]failed !!!", #METHOD, #FUNC_NAME);										 \
				return false;}

	#define	REGISTER_WEB_DEFAULT( METHOD, FUNC_CALLBACK)								 \
				if (!this->_register_web_default(METHOD, FUNC_CALLBACK)) {			 \
						ERROR_EX_LOG("register_web_default [method = %s]failed !!!",  #METHOD);										 \
				return false;}




	cweb_http_api_mgr g_web_http_api_mgr;
	cweb_http_api_mgr::cweb_http_api_mgr()
	: m_server(){}

#define PARSE_NAME_VALUE(VAR, NAME, TYPE) 							\
	if (data.isMember(#NAME) && data[#NAME].is##TYPE())			\
	{															\
		VAR.NAME = data[#NAME].as##TYPE();							\
	}															\
    else														\
	{															\
		WARNING_EX_LOG("not find or type  [var = %s]  ", #NAME); \
	}
	cweb_http_api_mgr::~cweb_http_api_mgr(){}
	bool cweb_http_api_mgr::init()
	{
		m_server.config.port = g_cfg.get_uint32(ECI_WebHttpWanPort);
		m_server.config.address = g_cfg.get_string(ECI_WebHttpWanIp);
		
		REGISTER_WEB_DEFAULT("OPTIONS", std::bind(&cweb_http_api_mgr::_handler_default_options, this, std::placeholders::_1, std::placeholders::_2));
		REGISTER_WEB_DEFAULT("GET", std::bind(&cweb_http_api_mgr::_handler_default_get, this, std::placeholders::_1, std::placeholders::_2));

		REGISTER_WEB_HANDLER("add_camera_info", "POST", std::bind(&cweb_http_api_mgr::_handler_add_camera_infos, this, std::placeholders::_1, std::placeholders::_2));
		REGISTER_WEB_HANDLER("camera_list/page=([0-9]+)&page_size=([0-9]+)", "GET", std::bind(&cweb_http_api_mgr::_handler_camera_list, this, std::placeholders::_1, std::placeholders::_2));
		REGISTER_WEB_HANDLER("delete_camera/camera_id=([0-9]+)", "GET", std::bind(&cweb_http_api_mgr::_handler_delete_camera, this, std::placeholders::_1, std::placeholders::_2));

		m_server.on_error = [](std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
			// Handle errors here
			// Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
		};

		
		return true;
	}
	void cweb_http_api_mgr::update()
	{}
	void cweb_http_api_mgr::destroy()
	{
		m_server.stop();
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	void cweb_http_api_mgr::startup()
	{
		std::promise<unsigned short> server_port;
		std::thread t_([this, &server_port]() 
		{
			// Start server
			m_server.start([&server_port](unsigned short port) 
			{
					SYSTEM_LOG("Web port = %u ", port);
				server_port.set_value(port);
			});
		});
		std::cout << "Web Server listening on port " << server_port.get_future().get() << std::endl;
		m_thread.swap(t_);
	}
	
	
	void cweb_http_api_mgr::_pthread_work()
	{
		


	}

	//void cweb_http_api_mgr::_send_message(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, /*uint32 result, const char* message,*/ const Json::Value& data)
	//{
	//	/*Json::Value value;
	//	value["result"] = result;
	//	value["data"] = data;
	//	value["message"] = message ? message : "";*/
	//	Json::StyledWriter swriter;
	//	std::string str = swriter.write(data);
	//	//Content-Type: application/json
	//	SimpleWeb::CaseInsensitiveMultimap header;
	//	header.emplace("Content-Type",  "application/json");
	//	// Content-Type: application/json; charset=utf-8
	//	header.emplace("Content-Type", "charset=utf-8");
	//	response->write(str, header);
	//}

	bool cweb_http_api_mgr::_register_web_handler(const char * function_name, const char * method, std::function<void(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response>response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request>request)> function)
	{
		std::string pathpre = std::string("^/") + g_cfg.get_string(ECI_WebPathPrefix) + function_name;
		NORMAL_EX_LOG("[pathpre = %s]", pathpre.c_str());
		return m_server.resource[std::string("^/") + g_cfg.get_string(ECI_WebPathPrefix) + function_name].insert(std::make_pair(method, function)).second;
	}

	bool cweb_http_api_mgr::_register_web_default(const char * method, std::function<void(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response>response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request>request)> function)
	{
		return m_server.default_resource.insert(std::make_pair(method, function)).second;
	}

	//uint32_t cweb_http_api_mgr::update_auth_info(uint64_t auth_id)
	//{
	//	// seach db 
	//	////////////////////////
	//	// 查询数据中数据
	//	enum EField
	//	{
	//		EF_auth_name = 0,
	//		EF_auth_pass,
	//		EF_start_time_stamp,
	//		EF_expire_time_stamp,
	//		EF_client_device,
	//		EF_offline_time,
	//	};

	//	g_sql_mgr.reset();
	//	g_sql_mgr << "SELECT `auth_name`, `auth_pass`, `start_time_stamp`, `expire_time_stamp`, `client_device`, `offline_time`  FROM `t_auth_info`  WHERE `id` = " << auth_id << "";
	//	cdb_query_result query_result;


	//	if (!g_global_db.send_query(query_result, g_sql_mgr))
	//	{
	//		WARNING_EX_LOG("g_global_db.send_query error !!!   [auth_id  = %u]", auth_id);
	//		//err.assign("sql query error");
	//		//reply.set_result(EEC_AuthRenderDb);
	//		return EGmDb;
	//	}

	//	if (!query_result.next_row())
	//	{
	//		WARNING_EX_LOG("not find [auth_id = %u]", auth_id);
	//		//err.assign("order not found");
	//		//reply.set_result(EEC_AuthRenderNotAuth);
	//		return  EGmNotFindAuthId;
	//	}

	//	const char * auth_name = query_result.get_col_value(EF_auth_name);
	//	const char* auth_pass = query_result.get_col_value(EF_auth_pass);
	//	int64  start_time_stamp = query_result.get_int64(EF_start_time_stamp);
	//	int64  expire_time_stamp = query_result.get_int64(EF_expire_time_stamp);
	//	const char* client_device = query_result.get_col_value(EF_client_device);
	//	uint64 offline_time = query_result.get_int64(EF_offline_time);
	//	uint32 index = g_wan_server.get_session_index(g_auth_mgr.get_session(auth_id));
	//	cwan_session* session_ptr = g_wan_server.get_session(index);
	//	if (!session_ptr)
	//	{
	//		WARNING_EX_LOG("[auth_id = %u] [index = %u]Not online", auth_id, index);
	//		return EGmNotOnline;
	//	}
	//	bool ret = session_ptr->is_used();
	//	if (false == ret)
	//	{
	//		WARNING_EX_LOG("[auth_id = %u][index = %u] Not online", auth_id, index);
	//		return EGmNotOnline;
	//	}
	//	AuthorizationInfo auth_info;
	//	auth_info.set_client_device(client_device);
	//	if (!client_device || strlen(client_device) < 5)
	//	{
	//		//WARNING_EX_LOG("[auth_id = %u][new client device = %s]", auth_id, msg.client_device().c_str());
	//		// save db device 

	//		std::string new_device = c_rand.rand_str(20);
	//		g_sql_mgr.reset();
	//		g_sql_mgr << "UPDATE `t_auth_info` SET `client_device` = '" << new_device << "' ";
	//		g_sql_mgr << "  WHERE `id` =  " << auth_id << " ";
	//		//cdb_query_result query_result;
	//		if (!g_global_db.excute(g_sql_mgr, ESqlError_CanIgnore))
	//		{
	//			WARNING_EX_LOG("g_global_db.send_query error !!!   [auth_id  = %u] update device failed !!!", auth_id);
	//			//	//err.assign("sql query error");
	//			//reply.set_result(EEC_AuthRenderDb);
	//			return -1;
	//		}
	//		auth_info.set_client_device(new_device);

	//	}
	//	
	//	CGUARD_REPLY(A2R_AuthorizationUpdate, session_ptr);
	//	auth_info.set_server_timestamp(std::time(NULL));
	//	auth_info.set_start_timestamp(start_time_stamp);
	//	auth_info.set_expire_timestamp(expire_time_stamp);
	//	auth_info.set_offline_time(offline_time);
	//	*reply.mutable_auth_info() = auth_info;
	//	reply.set_auth_name(auth_name);
	//	reply.set_auth_pass(auth_pass);
	//	return EGmSuccess;
	//}
	//

	//cresult_add_camera_info add_camera_infos(const AddCameraInfos & msg);
	cresult_add_camera_info cweb_http_api_mgr::add_camera_infos(const AddCameraInfos& msg)
	{
		//cresult_add_camera_info result;
		return g_camera_mgr.handler_add_camera_infos(msg); // g_global_logic_mgr.handler_web_create_render_app(msg );;
	}

	cresult_camera_list cweb_http_api_mgr::camera_list(uint32 page, uint32 page_size)
	{
		return g_camera_mgr.handler_camera_list(page, page_size);
	}

	uint32 cweb_http_api_mgr::delete_camera(uint32 camera_id)
	{
		return uint32();
	}

	 
}