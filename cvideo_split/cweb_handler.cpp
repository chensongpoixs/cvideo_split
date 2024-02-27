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
#include "cweb_guard_reply.h"
#include "crandom.h"

//#include "cguard_reply.h"
//#include "cmsg_base_id.h"
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
	//virtual   cresult_add_camera_info add_camera_infos(const AddCameraInfos & msg);
	void cweb_http_api_mgr::_handler_add_camera_infos(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		NORMAL_EX_LOG("");
		CWEB_GUARD_REPLY(response);
		std::string  content = request->content.string();
		 
		// pase json 
		Json::Reader reader;
		Json::Value new_data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), new_data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  "+ content+" failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());
			web_guard_reply.set_result(EWebJsonError);
			return;
		}
		if (!new_data.isMember("camera_infos") || !new_data["camera_infos"].isArray())
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());
			web_guard_reply.set_result(EWebJsonError);
			return;
		}
		//if (new_data.isArray())

		//create_render_app_struct create_app;
		AddCameraInfos camera_infos;
		Json::Value json_camera_infos = new_data["camera_infos"];
		try
		{
			for (Json::ArrayIndex i = 0; i < json_camera_infos.size(); ++i)
			{
				Json::Value data = json_camera_infos[i];
				uint32 camera_id = 0;
				std::string address;
				std::string	camera_name;
				uint32 port = 0;
				std::string url;
				std::string ip;
				PARSE_VALUE(camera_id, UInt);
				PARSE_VALUE(address, String);
				PARSE_VALUE(ip, String);
				PARSE_VALUE(camera_name, String);
				PARSE_VALUE(port, UInt);
				PARSE_VALUE(url, String);
				CameraInfo * camera_ptr = camera_infos.add_camera_infos();
				if (!camera_ptr)
				{
					WARNING_EX_LOG("web parse json alloc protobuff failed !!!");
					continue;
				}
				camera_ptr->set_ip(ip);
				camera_ptr->set_camera_id(camera_id);
				camera_ptr->set_address(address);
				camera_ptr->set_camera_name(camera_name);
				camera_ptr->set_port(port);
				camera_ptr->set_url(url);
			}
			 
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonParam, ret.c_str()); 
			web_guard_reply.set_result(EWebJsonParam);
			return;
		}
		cresult_add_camera_info result =   g_web_http_api_proxy.add_camera_infos(camera_infos);
		 
		web_guard_reply.set_result(  result.result);
		if (result.result == EWebSuccess)
		{
			for (size_t i = 0; i < result.camera_infos.camera_infos_size(); ++i)
			{
				Json::Value  CameraInfo;
				CameraInfo["index"] = result.camera_infos.camera_infos(i).index();
				CameraInfo["camera_id"] = result.camera_infos.camera_infos(i).camera_id();
				CameraInfo["address"] = result.camera_infos.camera_infos(i).address();
				CameraInfo["port"] = result.camera_infos.camera_infos(i).port();
				CameraInfo["url"] = result.camera_infos.camera_infos(i).url();
				CameraInfo["state"] = result.camera_infos.camera_infos(i).state();
				CameraInfo["ip"] = result.camera_infos.camera_infos(i).ip();
				reply["camera_infos"].append(CameraInfo);
			} 
		}
		
		//_send_message(response, resultdata);
		//_send_message(response, result.result, NULL, resultdata);
		 
	}

	void cweb_http_api_mgr::_handler_camera_list(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		//std::cout << "room_name = " << request->path_match[1] << ", username = " << request->path_match[2];
		CWEB_GUARD_REPLY(response);
		cresult_camera_list result = g_web_http_api_proxy.camera_list(std::atoi(request->path_match[1].str().c_str()), std::atoi(request->path_match[2].str().c_str()));
		web_guard_reply.set_result(result.result);
		reply["page_size"] = result.page_info.page_size();
		reply["page_number"] = result.page_info.page_number();
		reply["total_pages"] = result.page_info.total_pages();
		reply["total_elements"] = result.page_info.total_elements();

		 
		for (size_t i = 0; i < result.camera_infos.camera_infos_size(); ++i)
		{
			Json::Value  CameraInfo;
			CameraInfo["index"] = result.camera_infos.camera_infos(i).index();
			CameraInfo["camera_id"] = result.camera_infos.camera_infos(i).camera_id();
			CameraInfo["address"] = result.camera_infos.camera_infos(i).address();
			CameraInfo["camera_name"] = result.camera_infos.camera_infos(i).camera_name();
			CameraInfo["port"] = result.camera_infos.camera_infos(i).port();
			CameraInfo["url"] = result.camera_infos.camera_infos(i).url();
			CameraInfo["ip"] = result.camera_infos.camera_infos(i).ip();
			CameraInfo["state"] = result.camera_infos.camera_infos(i).state();
			reply["camera_infos"].append(CameraInfo);
		}
		if (result.camera_infos.camera_infos_size() <= 0)
		{
			Json::Value  CameraInfo; 
			reply["camera_infos"].append(CameraInfo);
		}
		 
		// reply --> json
	}

	void cweb_http_api_mgr::_handler_delete_camera(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		uint32 result = g_web_http_api_proxy.delete_camera(std::atoi(request->path_match[1].str().c_str()));
		web_guard_reply.set_result(result);
	}

	void cweb_http_api_mgr::_handler_add_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		VideoSplitInfo video_split_info;

		std::string  content = request->content.string();

		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());
			web_guard_reply.set_result(EWebJsonError, ret);
			return;
		}
		 
		 
		try
		{
			if (!data.isMember("split_channel_name") || !data["split_channel_name"].isString())
			{
				web_guard_reply.set_result(EWebJsonParam, "split_channel_name ");
				return;
			}
			if (!data.isMember("split_channel_id") || !data["split_channel_id"].isString())
			{
				web_guard_reply.set_result(EWebJsonParam, "split_channel_id ");
				return;
			}
			if (!data.isMember("multicast_ip") || !data["multicast_ip"].isString())
			{
				web_guard_reply.set_result(EWebJsonParam, "multicast_ip ");
				return;
			}
			if (!data.isMember("multicast_port") || !data["multicast_port"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "multicast_port ");
				return;
			}
			if (!data.isMember("split_method") || !data["split_method"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "split_method ");
				return;
			}
			if (!data.isMember("lock_1080p") || !data["lock_1080p"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "lock_1080p ");
				return;
			}
			if (!data.isMember("overlay") || !data["overlay"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "overlay ");
				return;
			}
			if (!data.isMember("camera_group") || !data["camera_group"].isArray())
			{
				web_guard_reply.set_result(EWebJsonParam, "camera_group ");
				return;
			}
			Json::Value camera_group_json = data["camera_group"];
			/*if (camera_group_json.size() < 2 || camera_group_json.size () > 10)
			{
				std::string message = "camera_group [size = " + std::to_string(camera_group_json.size()) + "]";
				web_guard_reply.set_result(EWebVideoSplitCameraGroupNum, message);
				return;
			}*/
			for (Json::ArrayIndex i = 0; i < camera_group_json.size(); ++i)
			{
				if (!camera_group_json[i].isMember("camera_id") || !camera_group_json[i]["camera_id"].isUInt())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  camera_id ");
					return;
				}
				if (!camera_group_json[i].isMember("index") || !camera_group_json[i]["index"].isUInt())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  index ");
					return;
				}
				if (!camera_group_json[i].isMember("x") || !camera_group_json[i]["x"].isDouble())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  x ");
					return;
				}
				if (!camera_group_json[i].isMember("y") || !camera_group_json[i]["y"].isDouble())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  y ");
					return;

				}
				if (!camera_group_json[i].isMember("w") || !camera_group_json[i]["w"].isDouble())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  w ");
					return;
				}
				if (!camera_group_json[i].isMember("h") || !camera_group_json[i]["h"].isDouble())
				{
					web_guard_reply.set_result(EWebJsonParam, "camera_group  h ");
					return;
				}
				CameraGroup * camera_group_ptr = video_split_info.add_camera_group();
				if (!camera_group_ptr)
				{
					web_guard_reply.set_result(EWebWait, "alloc camera_group  failed !!! ");
					return;
				}
				camera_group_ptr->set_index(camera_group_json[i]["index"].asUInt());
				camera_group_ptr->set_camera_id(camera_group_json[i]["camera_id"].asUInt());
				camera_group_ptr->set_x(camera_group_json[i]["x"].asDouble());
				camera_group_ptr->set_y(camera_group_json[i]["y"].asDouble());
				camera_group_ptr->set_w(camera_group_json[i]["w"].asDouble());
				camera_group_ptr->set_h(camera_group_json[i]["h"].asDouble());
			}
			video_split_info.set_split_channel_name(data["split_channel_name"].asString());
			video_split_info.set_split_channel_id(data["split_channel_id"].asString());
			video_split_info.set_multicast_ip(data["multicast_ip"].asString());
			video_split_info.set_multicast_port(data["multicast_port"].asUInt());
			video_split_info.set_split_method(static_cast<ESplitMethod>(data["split_method"].asUInt()));
			video_split_info.set_lock_1080p(data["lock_1080p"].asUInt());
			video_split_info.set_overlay(data["overlay"].asUInt()); 

			// out split video width and height
			if (!data.isMember("out_video_width") || !data["out_video_width"].isUInt() || !data.isMember("out_video_height") || !data["out_video_height"].isUInt())
			{
				//web_guard_reply.set_result(EWebJsonParam, "overlay ");
				//video_split_info.set_out_video_width(1080);
				video_split_info.set_out_video(1);
			}
			else
			{
				video_split_info.set_out_video_width(data["out_video_width"].asUInt());
				video_split_info.set_out_video_height(data["out_video_height"].asUInt());
			}
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonParam, ret.c_str()); 
			web_guard_reply.set_result(EWebJsonParam);
			return;
		}


		cresult_add_video_split result = g_web_http_api_proxy.add_video_split(video_split_info);
		web_guard_reply.set_result(result.result);

		Json::Value reply_video_split;
		if (result.result == EWebSuccess)
		{
			
			reply_video_split["id"] = result.video_split_info.id();
			reply_video_split["split_channel_name"] = result.video_split_info.split_channel_name();
			reply_video_split["split_channel_id"] = result.video_split_info.split_channel_id();
			reply_video_split["multicast_ip"] = result.video_split_info.multicast_ip();
			reply_video_split["multicast_port"] = result.video_split_info.multicast_port();
			reply_video_split["split_method"] = result.video_split_info.split_method();
			reply_video_split["lock_1080p"] = result.video_split_info.lock_1080p();
			reply_video_split["overlay"] = result.video_split_info.overlay();
			reply_video_split["out_video_width"] = result.video_split_info.out_video_width();
			reply_video_split["out_video_height"] = result.video_split_info.out_video_height();
			
			for (size_t i = 0; i < result.video_split_info.camera_group_size(); ++i)
			{
				Json::Value camera_group_reply;
				camera_group_reply["camera_id"] = result.video_split_info.camera_group(i).camera_id();
				camera_group_reply["index"] = result.video_split_info.camera_group(i).index();
				camera_group_reply["x"] = result.video_split_info.camera_group(i).x();
				camera_group_reply["y"] = result.video_split_info.camera_group(i).y();
				camera_group_reply["w"] = result.video_split_info.camera_group(i).w();
				camera_group_reply["h"] = result.video_split_info.camera_group(i).h();
				reply_video_split["camera_group"].append(camera_group_reply);
			}
		 }

		reply["data"] = reply_video_split;



	}
	void cweb_http_api_mgr::_handler_get_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		cresult_get_video_split result = g_web_http_api_proxy.get_video_split( request->path_match[1].str()  );
		web_guard_reply.set_result(result.result);
		if (result.result == EWebSuccess)
		{
			Json::Value  video_split_info;
			reply["video_split_infos"]["id"] = result.video_split_info.id();
			reply["video_split_infos"]["split_channel_name"] = result.video_split_info.split_channel_name();
			reply["video_split_infos"]["split_channel_id"] = result.video_split_info.split_channel_id();
			reply["video_split_infos"]["multicast_ip"] = result.video_split_info.multicast_ip();
			reply["video_split_infos"]["multicast_port"] = result.video_split_info.multicast_port();
			reply["video_split_infos"]["split_method"] = result.video_split_info.split_method();
			reply["video_split_infos"]["lock_1080p"] = result.video_split_info.lock_1080p();
			reply["video_split_infos"]["overlay"] = result.video_split_info.overlay();
			reply["video_split_infos"]["split_method"] = result.video_split_info.split_method();
			reply["video_split_infos"]["out_video_width"] = result.video_split_info.out_video_width();
			reply["video_split_infos"]["out_video_height"] = result.video_split_info.out_video_height();
			for (size_t j = 0; j < result.video_split_info.camera_group_size(); ++j)
			{
				Json::Value  CameraInfo;
				CameraInfo["index"] = result.video_split_info.camera_group(j).index();
				CameraInfo["camera_id"] = result.video_split_info.camera_group(j).camera_id();
				CameraInfo["x"] = result.video_split_info.camera_group(j).x();
				CameraInfo["y"] = result.video_split_info.camera_group(j).y();
				CameraInfo["w"] = result.video_split_info.camera_group(j).w();
				CameraInfo["h"] = result.video_split_info.camera_group(j).h();
				reply["video_split_infos"]["camera_group"].append(CameraInfo);
			}
			 
		}
	}


	void cweb_http_api_mgr::_handler_video_split_list(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		cresult_video_split_list result = g_web_http_api_proxy.video_split_list(std::atoi(request->path_match[1].str().c_str()), std::atoi(request->path_match[2].str().c_str()));
		web_guard_reply.set_result(result.result);
		reply["page_size"] = result.page_info.page_size();
		reply["page_number"] = result.page_info.page_number();
		reply["total_pages"] = result.page_info.total_pages();
		reply["total_elements"] = result.page_info.total_elements();


		for (size_t i = 0; i < result.video_split_infos.size(); ++i)
		{
			Json::Value  video_split_info;
			video_split_info["id"] = result.video_split_infos[i].id();
			video_split_info["split_channel_name"] = result.video_split_infos[i].split_channel_name();
			video_split_info["split_channel_id"] = result.video_split_infos[i].split_channel_id();
			video_split_info["multicast_ip"] = result.video_split_infos[i].multicast_ip();
			video_split_info["multicast_port"] = result.video_split_infos[i].multicast_port();
			video_split_info["split_method"] = result.video_split_infos[i].split_method();
			video_split_info["lock_1080p"] = result.video_split_infos[i].lock_1080p();
			video_split_info["overlay"] = result.video_split_infos[i].overlay();
			video_split_info["split_method"] = result.video_split_infos[i].split_method();
			video_split_info["out_video_width"] = result.video_split_infos[i].out_video_width();
			video_split_info["out_video_height"] = result.video_split_infos[i].out_video_height();
			for (size_t j = 0; j < result.video_split_infos[i].camera_group_size(); ++j)
			{
				Json::Value  CameraInfo;
				CameraInfo["index"] = result.video_split_infos[i].camera_group(j).index();
				//NORMAL_EX_LOG("index = %u", result.video_split_infos[i].camera_group(j).index());
				CameraInfo["camera_id"] = result.video_split_infos[i].camera_group(j).camera_id();
				CameraInfo["x"] = result.video_split_infos[i].camera_group(j).x();
				CameraInfo["y"] = result.video_split_infos[i].camera_group(j).y();
				CameraInfo["w"] = result.video_split_infos[i].camera_group(j).w();
				CameraInfo["h"] = result.video_split_infos[i].camera_group(j).h();
				//NORMAL_EX_LOG("CameraInfo = %s", CameraInfo.asCString());
				video_split_info["camera_group"].append(CameraInfo);
			}
			reply["video_split_infos"].append(video_split_info);
		}
		if (result.video_split_infos.size() <= 0)
		{
			Json::Value  CameraInfo;
			reply["video_split_infos"].append(CameraInfo);
		}
	}

 
	void cweb_http_api_mgr :: _handler_delete_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		uint32 result = g_web_http_api_proxy.delete_video_split((request->path_match[1].str()));
		web_guard_reply.set_result(result);
	}

	void cweb_http_api_mgr::_handler_cmd_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);
		uint32 result = g_web_http_api_proxy.cmd_video_split( (request->path_match[1].str() ), std::atoi(request->path_match[2].str().c_str()));
		web_guard_reply.set_result(result);
	}
	void cweb_http_api_mgr::_handler_modify_video_split(std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request)
	{
		CWEB_GUARD_REPLY(response);

		std::string  content = request->content.string();

		// pase json 
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)content.c_str(), (const char*)content.c_str() + content.size(), data))
		{
			WARNING_EX_LOG("parse  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());
			web_guard_reply.set_result(EWebJsonError, ret);
			return;
		}

		
		VideoSplitOsd video_osd;
		try
		{
			if (!data.isMember("split_channel_id") || !data["split_channel_id"].isString())
			{
				web_guard_reply.set_result(EWebJsonParam, "split_channel_id ");
				return;
			}
			if (!data.isMember("out_video_width") || !data["out_video_width"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "out_video_width ");
				return;
			}
			if (!data.isMember("out_video_height") || !data["out_video_height"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "out_video_height ");
				return;
			}
			if (!data.isMember("txt") || !data["txt"].isString())
			{
				web_guard_reply.set_result(EWebJsonParam, "txt ");
				return;
			}
			 
			if (!data.isMember("fontsize") || !data["fontsize"].isUInt())
			{
				web_guard_reply.set_result(EWebJsonParam, "fontsize ");
				return;
			}
			if (!data.isMember("x") || !data["x"].isDouble())
			{
				web_guard_reply.set_result(EWebJsonParam, "x ");
				return;
			}
			if (!data.isMember("y") || !data["y"].isDouble())
			{
				web_guard_reply.set_result(EWebJsonParam, "y ");
				return;
			}

			video_osd.set_split_channel_id(data["split_channel_id"].asString());
			video_osd.set_txt( data["txt"].asString());
			video_osd.set_fontsize(data["fontsize"].asUInt());
			video_osd.set_video_width(data["out_video_width"].asUInt());
			video_osd.set_video_height(data["out_video_height"].asUInt());
			video_osd.set_x( data["x"].asDouble());
			video_osd.set_y( data["y"].asDouble());
		}
		catch (const std::exception&)
		{
			WARNING_EX_LOG("parse value  [payload = %s] failed !!!", content.c_str());
			std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonParam, ret.c_str()); 
			web_guard_reply.set_result(EWebJsonParam);
			return;
		}


		cresult_video_split_osd result = g_web_http_api_proxy.modify_video_split(video_osd);
		web_guard_reply.set_result(result.result);
		if (result.result == EWebSuccess)
		{
			reply["split_channel_id"] = result.video_split_osd.split_channel_id();
			reply["out_video_width"] = result.video_split_osd.video_width();
			reply["out_video_height"] = result.video_split_osd.video_height();
			reply["txt"] = result.video_split_osd.txt();
			reply["fontsize"] = result.video_split_osd.fontsize();
			reply["x"] = result.video_split_osd.x();
			reply["y"] = result.video_split_osd.y();
		}
	}

}