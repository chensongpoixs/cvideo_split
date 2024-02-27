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
		virtual   cresult_add_camera_info add_camera_infos(const AddCameraInfos & msg);
		virtual   cresult_camera_list camera_list(uint32 page, uint32 page_size);
		virtual	  uint32			  delete_camera(uint32 camera_id);
		 

		virtual  cresult_add_video_split add_video_split( const VideoSplitInfo& video_split_info);
		virtual  cresult_get_video_split get_video_split(const std::string& channel_name);

		virtual cresult_video_split_list video_split_list(uint32 page, uint32 page_size);
		virtual uint32				delete_video_split(const std::string & channel_name/*uint32 id*/);


		virtual uint32		cmd_video_split(const std::string& channel_name/*uint32 id*/, uint32 cmd);
		virtual cresult_video_split_osd		modify_video_split(const VideoSplitOsd& video_osd);
	private:
		void _handler_default_options(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_default_get(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);

		void _handler_add_camera_infos(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_camera_list(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_delete_camera(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		
		// video split -->
		void _handler_add_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_get_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_video_split_list(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_delete_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		
		void _handler_cmd_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);
		void _handler_modify_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request);

	private:
		void _pthread_work();

	private:
		//void _send_message(uint32 result);
		// uint32 result= 0, const char * message = NULL,
		//void _send_message(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response,  const Json::Value & data = Json::Value());
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