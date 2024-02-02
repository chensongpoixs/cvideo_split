/***********************************************************************************************
created: 		2019-05-01

author:			chensong

purpose:		ccfg

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
#include "ccfg.h"
//#include "cserver_cfg.h"
#include "cweb_http_api_mgr.h"
#include <iostream>
#include "ccfg.h"
#include <thread>
#include "cweb_http_api_proxy.h"
//#include <json.h>
#include <json/json.h>
#include <cinttypes>
#include "chttp_code.h"
//#include "cserver_msg_header.h"
#include "cweb_http_struct.h"

#include "crandom.h"

//#include "cguard_reply.h"
#include "cmsg_base_id.h"
//#include "cserver_cfg.h"
#include "ccfg.h"
#include "cweb_http_api_mgr.h"


namespace chen {


#define PARSE_VALUE(VAR, TYPE) 							\
	if (data.isMember(#VAR) && data[#VAR].is##TYPE())			\
	{															\
		VAR = data[#VAR].as##TYPE();							\
	}															\
    else														\
	{															\
		WARNING_EX_LOG("not find or type  [var = %s]  ", #VAR); \
	}

#define PARSE_NAME_VALUE(VAR, NAME, TYPE) 							\
	if (data.isMember(#NAME) && data[#NAME].is##TYPE())			\
	{															\
		VAR.NAME = data[#NAME].as##TYPE();							\
	}															\
    else														\
	{															\
		WARNING_EX_LOG("not find or type  [var = %s]  ", #NAME); \
	}


	void cweb_http_api_mgr::_handler_default_options(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		// TODO@chensong 2024-01-05 神奇 web端请求POST时会先请求是否支持发送OPTIONS方法 然后返回204的错误码就可以啦 ^_^
		response->write(SimpleWeb::StatusCode::success_no_content);
	}

	void cweb_http_api_mgr::_handler_default_get(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / request->path);
			// Check if path is within web_root_path
			if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
				!std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
				throw std::invalid_argument("path must be within root path");
			if (boost::filesystem::is_directory(path))
				path /= "index.html";

			SimpleWeb::CaseInsensitiveMultimap header;



			auto ifs = std::make_shared<std::ifstream>();
			ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

			if (*ifs) {
				auto length = ifs->tellg();
				ifs->seekg(0, std::ios::beg);

				header.emplace("Content-Length", to_string(length));
				response->write(header);

				// Trick to define a recursive function within this scope (for example purposes)
				class FileServer {
				public:
					static void read_and_send(const std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response>& response, const std::shared_ptr<std::ifstream>& ifs) {
						// Read and send 128 KB at a time
						static std::vector<char> buffer(131072); // Safe when server is running on one thread
						std::streamsize read_length;
						if ((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0) {
							response->write(&buffer[0], read_length);
							if (read_length == static_cast<std::streamsize>(buffer.size())) {
								response->send([response, ifs](const SimpleWeb::error_code& ec) {
									if (!ec)
										read_and_send(response, ifs);
									else
										std::cerr << "Connection interrupted" << std::endl;
									});
							}
						}
					}
				};
				FileServer::read_and_send(response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception& e) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
		}
	}
	void cweb_http_api_mgr::_handler_create_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		 
		std::string  content = request->content.string();
		 
		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  "+ content+" failed !!! ";
			_send_message(response, EWebJsonError, ret.c_str());
			 
			return;
		}

		 

		create_render_app_struct create_app;

		try
		{
			PARSE_NAME_VALUE(create_app, render_id, String);
			PARSE_NAME_VALUE(create_app, project_id, String);
			PARSE_NAME_VALUE(create_app, scene_id, String);
			PARSE_NAME_VALUE(create_app, user_name, String);
			PARSE_NAME_VALUE(create_app, auto_restart, UInt);
			PARSE_NAME_VALUE(create_app, signaling_ip, String);
			PARSE_NAME_VALUE(create_app, signaling_port, UInt); 
			PARSE_NAME_VALUE(create_app, media_ip, String);
			PARSE_NAME_VALUE(create_app, media_port, UInt);
			PARSE_NAME_VALUE(create_app, app_render_type, UInt);
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonParam, ret.c_str()); 
			return;
		} 
		cresult_app_info result =   g_web_http_api_proxy.create_render_app(create_app);
		Json::Value resultdata;
		if (result.result == EWebSuccess)
		{
			/*resultdata["app_id"] = result.app_render.app_id();
			resultdata["app_name"] = result.app_render.app_name();
			resultdata["media_ip"] = result.app_render.media_ip();
			resultdata["media_port"] = result.app_render.media_port();
			resultdata["auto_restart"] = result.app_render.auto_restart();
			resultdata["app_render_type"] = result.app_render.app_render_type();
			resultdata["user_id"] = result.app_render.user_id();
			resultdata["default_project_id"] = result.app_render.default_project_id();
			resultdata["default_scene_content_id"] = result.app_render.default_scene_content_id();
			resultdata["default_scene_project_name"] = result.app_render.default_scene_project_name();
			resultdata["app_status"] = result.app_render.app_status();
			resultdata["signaling_ip"] = result.app_render.signaling_ip();
			resultdata["signaling_port"] = result.app_render.signaling_port();*/
		}
		
		_send_message(response, result.result, NULL, resultdata);
		 
	}

	void cweb_http_api_mgr::_handler_update_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		std::string  content = request->content.string();

		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonError, ret.c_str());

			return;
		}



		update_render_app_struct update_app;

		try
		{
			PARSE_NAME_VALUE(update_app, app_id, String);
			PARSE_NAME_VALUE(update_app, project_id, String);
			PARSE_NAME_VALUE(update_app, scene_id, String);
			PARSE_NAME_VALUE(update_app, user_name, String);
			PARSE_NAME_VALUE(update_app, auto_restart, UInt);
			PARSE_NAME_VALUE(update_app, signaling_ip, String);
			PARSE_NAME_VALUE(update_app, signaling_port, UInt);
			PARSE_NAME_VALUE(update_app, media_ip, String);
			PARSE_NAME_VALUE(update_app, media_port, UInt);
			PARSE_NAME_VALUE(update_app, app_render_type, UInt);
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonParam, ret.c_str());
			return;
		}
		cresult_app_info result  = g_web_http_api_proxy.update_render_app(update_app);
		Json::Value resultdata;
		if (result.result == EWebSuccess)
		{
			/*resultdata["app_id"] = result.app_render.app_id();
			resultdata["app_name"] = result.app_render.app_name();
			resultdata["media_ip"] = result.app_render.media_ip();
			resultdata["media_port"] = result.app_render.media_port();
			resultdata["auto_restart"] = result.app_render.auto_restart();
			resultdata["app_render_type"] = result.app_render.app_render_type();
			resultdata["user_id"] = result.app_render.user_id();
			resultdata["default_project_id"] = result.app_render.default_project_id();
			resultdata["default_scene_content_id"] = result.app_render.default_scene_content_id();
			resultdata["default_scene_project_name"] = result.app_render.default_scene_project_name();
			resultdata["app_status"] = result.app_render.app_status();
			resultdata["signaling_ip"] = result.app_render.signaling_ip();
			resultdata["signaling_port"] = result.app_render.signaling_port();*/
		}

		_send_message(response, result.result, NULL, resultdata);
	}

	void cweb_http_api_mgr::_handler_delete_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		std::string  content = request->content.string();

		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonError, ret.c_str());

			return;
		}



		std::string app_id;

		try
		{
			PARSE_VALUE(  app_id, String); 
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonParam, ret.c_str());
			return;
		}
		uint32_t ret = g_web_http_api_proxy.delete_render_app(app_id);
		_send_message(response, ret);
	}

	void cweb_http_api_mgr::_handler_cmd_render_app(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response>  response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request>  request)
	{
		std::string  content = request->content.string();

		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonError, ret.c_str());

			return;
		} 
		cmd_render_app_struct cmd_app;
		try
		{
			PARSE_NAME_VALUE(cmd_app, app_id, String);
			PARSE_NAME_VALUE(cmd_app, cmd, UInt);
			PARSE_NAME_VALUE(cmd_app, app_show, UInt);
			PARSE_NAME_VALUE(cmd_app, app_render_type, UInt);
			 
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			_send_message(response, EWebJsonParam, ret.c_str());
			return;
		}
		//

		//查询数据库数据
	 

		uint32_t ret =   g_web_http_api_proxy.cmd_render_app(cmd_app);
		_send_message(response, ret);
	}


}