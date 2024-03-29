/********************************************************************
created:	2021-11-24

author:		chensong

purpose:	Location

*********************************************************************/

#ifndef _C_WEB_API_PROXY_H_
#define _C_WEB_API_PROXY_H_

#include <memory>
#include <string>
#include <utility>
#include "cweb_http_api_interface.h"
#include "server_http.hpp"
#include <future>

// Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// Added for the default_resource example
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <json/json.h>
#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif
#include <vector>
//#include "croom.h"
#include "cweb_http_struct.h"
namespace chen {


	enum CWebHttpApiCodc
	{


	};


	class cweb_http_api_mgr : public cweb_http_api_interface
	{
	public:
		cweb_http_api_mgr();
		virtual ~cweb_http_api_mgr();


	public:
		bool init();
		void update();
		void destroy();



		void startup();
	public:
		//virtual	  uint32_t update_auth_info(uint64_t auth_id);
		virtual   cresult_app_info create_render_app(const create_render_app_struct & msg);
		virtual   cresult_app_info update_render_app(const update_render_app_struct& msg);
		virtual   uint32_t delete_render_app(const std::string& app_id);
		virtual   uint32_t cmd_render_app(const cmd_render_app_struct& msg);
		//virtual std::vector< croom_info>   get_all_room();
		//virtual std::vector< chen::cuser_info>   get_room_info(const std::string& room_name );
		//virtual uint32_t  kick_room_username(const std::string& room_name, const std::string & user_name);
		//virtual uint32_t  add_while_room_username(const std::string& room_name, const std::string & user_name);
		//virtual uint32_t  delete_while_room_username(const std::string& room_name, const std::string & user_name);
	
	
	private:
		void _handler_default_options(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_default_get(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);

		void _handler_create_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_update_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_delete_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);

		void _handler_cmd_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);

	private:
		void _pthread_work();

	private:
		//void _send_message(uint32 result);
		void _send_message(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, uint32 result= 0, const char * message = NULL, const Json::Value & data = Json::Value());
	private:
		bool _register_web_handler(const char * function_name, const char * method, std::function<void (std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)> function);
		bool _register_web_default(const char * fun_name, std::function<void(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)> function);
		//m_server.resource["^/cmd_render_app"]["POST"]
	private:
		SimpleWeb::Server<SimpleWeb::HTTP>	m_server;

	std::thread							m_thread;
	};
	extern cweb_http_api_mgr g_web_http_api_mgr;
}



#endif // _C_WEB_API_PROXY_H_