/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	video_split mgr

*********************************************************************/
#include "cvideo_split_info_mgr.h"
#include "clog.h"
#include "ccfg.h"
#include "cvideo_split_mgr.h"
#include "chttp_code.h"
#include "cvms_device.h"
namespace chen {


	static std::unordered_set<std::string>    g_global_channel_id;
	static std::unordered_map<uint32, std::unordered_map<std::string, std::string>>  g_global_video_channel;


	static const char* g_video_split_json_name = "video_split_list.json";
	cvideo_split_info_mgr g_video_split_info_mgr;
	bool cvideo_split_info_mgr::init()
	{
		_load_video_split_config();


		for (VIDEO_SPLIT_INFO_MAP::iterator iter = m_video_split_info_map.begin();
			iter != m_video_split_info_map.end(); ++iter)
		{
			g_vms_device_mgr.update_device_info(iter->second.id(), iter->second.split_channel_id(), iter->second.split_channel_name(), 0);
		}


		return true;
	}
	void cvideo_split_info_mgr::update(uint32 uDelta)
	{
		if (m_data_type != EDataNone)
		{
			_sync_save_video_split_data();
			m_data_type = EDataNone;
		}
	}
	void cvideo_split_info_mgr::destroy()
	{
		if (m_data_type != EDataNone)
		{
			_sync_save_video_split_data();
		}
		m_video_split_info_map.clear();
	}
	const VideoSplitInfo* cvideo_split_info_mgr::get_video_split_info(const std::string& channel_name /*uint32 id*/) const
	{
		// TODO: 在此处插入 return 语句
		VIDEO_SPLIT_INFO_MAP::const_iterator const_iterator =  m_video_split_info_map.find(channel_name);
		if (const_iterator != m_video_split_info_map.end())
		{
			return &const_iterator->second;
		}
		return NULL;

	}
	VideoSplitInfo* cvideo_split_info_mgr::get_video_split_info(const std::string& channel_name/*uint32 id*/)
	{
		// TODO: 在此处插入 return 语句
		VIDEO_SPLIT_INFO_MAP:: iterator iterator = m_video_split_info_map.find(channel_name);
		if (iterator != m_video_split_info_map.end())
		{
			return &iterator->second;
		}
		return NULL;
	}
	void cvideo_split_info_mgr::_load_video_split_config()
	{
		std::string video_split_name = g_cfg.get_string(ECI_DataPath) + "/" + g_video_split_json_name;
		FILE* read_file_ptr = ::fopen(video_split_name.c_str(), "rb");
		if (!read_file_ptr)
		{

			WARNING_EX_LOG("not open data file [%s] failed !!!", video_split_name.c_str());
			return;
		}

		// 数据的大小
		// 移动到文件末尾
		::fseek(read_file_ptr, 0, SEEK_END);
		// 获取当前位置，即文件大小
		uint32 size = ::ftell(read_file_ptr);
		::fseek(read_file_ptr, 0, SEEK_SET);


		std::string video_split_data;
		video_split_data.resize(size + 1);
		size_t read_size = ::fread((void*)video_split_data.data(), 1, size, read_file_ptr);


		if (read_file_ptr)
		{
			::fclose(read_file_ptr);
			read_file_ptr = NULL;
		}
		/// parse json 
		_parse_json_data(video_split_data);
	}
	cresult_add_video_split cvideo_split_info_mgr::handler_web_add_video_split(  VideoSplitInfo& video_split_info)
	{
		cresult_add_video_split result;
		VIDEO_SPLIT_INFO_MAP::iterator iter = m_video_split_info_map.find(video_split_info.split_channel_id());
		

		

		if (iter != m_video_split_info_map.end())
		{
			if (g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()] 
				!= video_split_info.split_channel_id() && g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()].size() > 2)
			{
				WARNING_EX_LOG("[global stsr = %s][ split channel = %s]", g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()].c_str(), video_split_info.split_channel_id().c_str());
				result.result = EWebVideoChannelMulticastAddress;
				return result;
			}
			else
			{
				// 删除信息
				if (iter->second.multicast_ip() != video_split_info.multicast_ip() ||
					iter->second.multicast_port() != video_split_info.multicast_port())
				{
					g_global_video_channel[iter->second.multicast_port()][iter->second.multicast_ip()] = "";
				}
			}
			if (video_split_info.out_video())
			{
				video_split_info.set_out_video_width(iter->second.out_video_width());
				video_split_info.set_out_video_height(iter->second.out_video_height());
			}
			video_split_info.set_id(iter->second.id());
			 
			if (video_split_info.split_channel_id().size()> 0&& video_split_info.split_channel_id() != iter->second.split_channel_id())
			{
				if (!g_global_channel_id.insert(video_split_info.split_channel_id()).second)
				{
					result.result = EWebFindVideoSplitId;
					WARNING_EX_LOG("insert channel id = [%s] failed !!!", video_split_info.split_channel_id().c_str());
					return result;

				}
				else
				{
					g_global_channel_id.erase(iter->second.split_channel_id());
				}
			}
			else
			{
				video_split_info.set_split_channel_id(iter->second.split_channel_id());
			}
			{
				for (size_t i = 0; i < video_split_info.camera_group_size(); ++i)
				{
					bool find = false;
					for (size_t j = 0; j < iter->second.camera_group_size(); ++j)
					{
						if (video_split_info.camera_group(i).index() == iter->second.camera_group(j).index())
						{
							find = true;
							*iter->second.mutable_camera_group(j) = video_split_info.camera_group(i);
						}
					}
					if (!find)
					{
						CameraGroup * camera_group_ptr = iter->second.add_camera_group();
						if (camera_group_ptr)
						{
							*camera_group_ptr = video_split_info.camera_group(i);
						}
					}
				}
				*video_split_info.mutable_camera_group() = iter->second.camera_group();
				iter->second.set_camera_groups_size(video_split_info.camera_groups_size());
			}
			

			g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()] = video_split_info.split_channel_id();

			*video_split_info.mutable_osd_info() = iter->second.osd_info();
			result.video_split_info = video_split_info;
			iter->second = video_split_info;
			m_data_type = EDataLoad;
			return result;
		}

		if (!g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()].empty())
		{
			result.result = EWebVideoChannelMulticastAddress;
			return result;
		}
		if (video_split_info.out_video())
		{
			video_split_info.set_out_video_width(1920);
			video_split_info.set_out_video_height(1080);
		}
		if (!g_global_channel_id.insert(video_split_info.split_channel_id()).second)
		{
			result.result = EWebFindVideoSplitId;
			WARNING_EX_LOG("insert channel id = [%s] failed !!!", video_split_info.split_channel_id().c_str());
			return result;
		}
		std::pair< VIDEO_SPLIT_INFO_MAP::iterator, bool> pi = m_video_split_info_map.insert(std::make_pair(video_split_info.split_channel_id(), video_split_info));
		if (!pi.second)
		{
			result.result = EWebWait;
			WARNING_EX_LOG("web video split map insert  [id = %s] failed !!!", video_split_info.split_channel_id().c_str());
			return result;
		}
		g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()] = video_split_info.split_channel_id();

		pi.first->second.set_id(m_video_split_index++);
		result.video_split_info = pi.first->second;
		m_data_type = EDataLoad;
		return result;
	}
	cresult_get_video_split cvideo_split_info_mgr::handler_web_get_video_split(const std::string& channel_id) const
	{
		cresult_get_video_split result;
		VIDEO_SPLIT_INFO_MAP::const_iterator const_iterator=  m_video_split_info_map.find(channel_id);
		if (const_iterator == m_video_split_info_map.end())
		{
			result.result = EWebNotFindVideoSplitId;
			WARNING_EX_LOG("not find [channel_id = %s]", channel_id.c_str());
			return result;
		}
		result.video_split_info = const_iterator->second;
		return result;
	}
	cresult_video_split_list cvideo_split_info_mgr::handler_web_video_split_list(uint32 page, uint32 page_size)
	{
		cresult_video_split_list result;




		result.page_info.set_total_pages((m_video_split_info_map.size() / page_size) + ((m_video_split_info_map.size() % page_size) > 0 ? 1 : 0));
		result.page_info.set_total_elements(m_video_split_info_map.size());
		result.page_info.set_page_number(page);
		int32 show_page_size = 0;
		int32 start_index = (page_size * (page));
		if (m_video_split_info_map.size() >= start_index)
		{
			show_page_size = (m_video_split_info_map.size() - start_index) > page_size ? page_size : (m_video_split_info_map.size() - start_index );
			result.page_info.set_page_size(show_page_size);
			//CAMERA_INFO_MAP::const_iterator iter = m_camera_info_map.find(camera_id);
			for (VIDEO_SPLIT_INFO_MAP::const_iterator iter = m_video_split_info_map.begin();
				iter != m_video_split_info_map.end(); ++iter)
			{
				if (start_index <= 0)
				{
					if (show_page_size <= 0)
					{
						return result;
					}
					/*CameraInfo* camera_info_ptr = result.video_split_infos.();
					if (!camera_info_ptr)
					{
						WARNING_EX_LOG("protobut alloc failed !!! ");
						continue;
					}
					*camera_info_ptr = iter->second;*/
					result.video_split_infos.push_back(iter->second);
					--show_page_size;
				}
				else
				{
					--start_index;
				}
			}
		}

		//result.page_info.set_total_elements();
		return result;
	}
	uint32 cvideo_split_info_mgr::handler_web_delete_video_split(const std::string& channel_id/*uint32 id*/)
	{
		VIDEO_SPLIT_INFO_MAP::iterator iter = m_video_split_info_map.find(channel_id);
		if (iter != m_video_split_info_map.end())
		{
			 if (iter->second.status() != 0)
			{
				WARNING_EX_LOG("delete failed channel_id = %s ,  open !!!", channel_id.c_str());
				g_video_split_mgr.handler_web_cmd_video_split(channel_id ,1 );
				//return EWebDeleteVideoChannelOpen;
			 }
				// TODO@chensong  20240606 删除信息
				 
				{
					g_global_video_channel[iter->second.multicast_port()][iter->second.multicast_ip()] = "";
				}
			 
			m_video_split_info_map.erase(iter);
			NORMAL_EX_LOG("delete [video_split_channel_id = %s]", channel_id.c_str());
			m_data_type = EDataLoad;
			//_sync_save_video_split_data();
			/*if (next_iter != m_camera_info_map.end())
			{
				WARNING_EX_LOG("=!======> [camera_id = %u][next_iter = %u]", camera_id, next_iter->first);
			}
			else
			{
				WARNING_EX_LOG("=======> camera_id = %u", camera_id);
			}*/
			return EWebSuccess;
		}
		WARNING_EX_LOG("not find [video_split_ channel_id  = %s]", channel_id.c_str());
		return EWebNotFindVideoSplitId;
	}
	cresult_video_split_osd	cvideo_split_info_mgr::handler_web_modify_video_split(const VideoSplitOsd& video_osd)
	{
		cresult_video_split_osd result;
		VIDEO_SPLIT_INFO_MAP::iterator iter = m_video_split_info_map.find(video_osd.split_channel_id());
		if (iter != m_video_split_info_map.end())
		{

			//m_video_split_info_map.erase(iter);
			NORMAL_EX_LOG("  [video_split_channel_id = %s]", video_osd.split_channel_id().c_str());
			OsdInfo* osd_info = iter->second.mutable_osd_info();;
			if (osd_info)
			{
				osd_info->set_font_text(video_osd.txt());
				osd_info->set_font_size(video_osd.fontsize());
				osd_info->set_x(video_osd.x());
				osd_info->set_y(video_osd.y());
			}
			//if (video_osd.width() != 0)
			{
				iter->second.set_out_video_width(video_osd.video_width());
			}
			//if (video_osd.height() != 0)
			{
				iter->second.set_out_video_height(video_osd.video_height());
			}
			//iter->second.mutable_osd_info
			m_data_type = EDataLoad;
			//_sync_save_video_split_data();
			/*if (next_iter != m_camera_info_map.end())
			{
				WARNING_EX_LOG("=!======> [camera_id = %u][next_iter = %u]", camera_id, next_iter->first);
			}
			else
			{
				WARNING_EX_LOG("=======> camera_id = %u", camera_id);
			}*/
			result.video_split_osd = video_osd;
			return result;
		}
		WARNING_EX_LOG("not find [video_split_ channel_id  = %s]", video_osd.split_channel_id().c_str());
		result.result = EWebNotFindVideoSplitId;
		return result;
	}
	void cvideo_split_info_mgr::_parse_json_data(const std::string& json_data)
	{
		Json::Reader reader;
		Json::Value data;

		if (!reader.parse((const char*)json_data.c_str(), (const char*)(json_data.c_str() + json_data.size()), data))
		{
			WARNING_EX_LOG("parse  [video_split_list = %s] failed !!!", json_data.c_str());
			//std::string ret = "parse  " + content + " failed !!! ";
			//_send_message(response, EWebJsonError, ret.c_str());

			return;
		}

		/*

		[
			 { "address": "192.168.1.34"},
			 {"address": "192.168.1.35"}

		]
		*/

		if (!data.isArray())
		{
			WARNING_EX_LOG("camera data not is arrays [%s] !!!", data.asCString());
			return;
		}

		for (uint32 i = 0; i < data.size(); ++i)
		{
			CameraInfo camera_info;
			if (!data[i].isMember("id") || !data[i]["id"].isUInt64())
			{
				WARNING_EX_LOG("config  warr id !!!");
				continue;
			}
			if (!data[i].isMember("split_channel_name") || !data[i]["split_channel_name"].isString())
			{
				WARNING_EX_LOG("config  warr split_channel_name !!!");
				continue;
			}
			if (!data[i].isMember("split_channel_id") || !data[i]["split_channel_id"].isString())
			{
				WARNING_EX_LOG("config  warr split_channel_id !!!");
				continue;
			}
			if (!data[i].isMember("multicast_ip") || !data[i]["multicast_ip"].isString())
			{
				WARNING_EX_LOG("config  warr multicast_ip !!!");
				continue;
			}
			if (!data[i].isMember("multicast_port") || !data[i]["multicast_port"].isUInt())
			{
				WARNING_EX_LOG("config  warr multicast_port !!!");
				continue;
			}
			if (!data[i].isMember("split_method") || !data[i]["split_method"].isUInt())
			{
				WARNING_EX_LOG("config  warr split_method !!!");
				continue;
			}
			if (!data[i].isMember("lock_1080p") || !data[i]["lock_1080p"].isUInt())
			{
				WARNING_EX_LOG("config  warr lock_1080p !!!");
				continue;
			}
			if (!data[i].isMember("overlay") || !data[i]["overlay"].isUInt())
			{
				WARNING_EX_LOG("config  warr overlay !!!");
				continue;
			}

			if (!data[i].isMember("out_video_width") || !data[i]["out_video_width"].isUInt())
			{
				WARNING_EX_LOG("config  warr out_video_width !!!");
				continue;
			}
			if (!data[i].isMember("out_video_height") || !data[i]["out_video_height"].isUInt())
			{
				WARNING_EX_LOG("config  warr out_video_height !!!");
				continue;
			}
			if (!data[i].isMember("camera_group") || !data[i]["camera_group"].isArray())
			{
				WARNING_EX_LOG("config  warr camera_group !!!");
				continue;
			}
			
			if (!data[i].isMember("osd_info") || !data[i]["osd_info"].isObject())
			{
				WARNING_EX_LOG("config  warr osd_info !!!");
				continue;
			}
			VideoSplitInfo video_split_info;
			video_split_info.set_id(data[i]["id"].asUInt64());
			video_split_info.set_split_channel_name(data[i]["split_channel_name"].asString());
			video_split_info.set_split_channel_id(data[i]["split_channel_id"].asString());


			if (!g_global_channel_id.insert(video_split_info.split_channel_id()).second)
			{
				WARNING_EX_LOG("global channel id [%s](channel_name =%s) insert failed !!!", video_split_info.split_channel_id().c_str(), video_split_info.split_channel_name().c_str());
			}
			video_split_info.set_multicast_ip(data[i]["multicast_ip"].asString());
			video_split_info.set_multicast_port(data[i]["multicast_port"].asUInt64());
			video_split_info.set_split_method(static_cast<ESplitMethod>(data[i]["split_method"].asUInt64()));
			video_split_info.set_lock_1080p(data[i]["lock_1080p"].asUInt64());
			video_split_info.set_overlay(data[i]["overlay"].asUInt64()); 
			video_split_info.set_out_video_width(data[i]["out_video_width"].asUInt64());
			video_split_info.set_out_video_height(data[i]["out_video_height"].asUInt64());
			
			// camera info
			Json::Value camera_infos_json = data[i]["camera_group"];

			_parse_camera_group(camera_infos_json, video_split_info);
			if (!data[i].isMember("camera_group_size") || !data[i]["camera_group_size"].isInt64())
			{
				video_split_info.set_camera_groups_size(video_split_info.camera_group_size());
			}
			else
			{
				video_split_info.set_camera_groups_size(data[i]["camera_group_size"].asInt64());
			}

			// OSD info
			Json::Value osd_json = data[i]["osd_info"];
			if (!osd_json.isMember("x") || !osd_json["x"].isDouble())
			{
				WARNING_EX_LOG("config  warr osd_info x  !!!");
				continue;
			}
			if (!osd_json.isMember("y") || !osd_json["y"].isDouble())
			{
				WARNING_EX_LOG("config  warr osd_info y  !!!");
				continue;
			}
			if (!osd_json.isMember("font_size") || !osd_json["font_size"].isUInt())
			{
				WARNING_EX_LOG("config  warr osd_info font_size  !!!");
				continue;
			}
			if (!osd_json.isMember("font_text") || !osd_json["font_text"].isString())
			{
				WARNING_EX_LOG("config  warr osd_info font_text  !!!");
				continue;
			}
			video_split_info.mutable_osd_info()->set_x(osd_json["x"].asDouble());
			video_split_info.mutable_osd_info()->set_y(osd_json["y"].asDouble());
			video_split_info.mutable_osd_info()->set_font_size(osd_json["font_size"].asUInt());
			video_split_info.mutable_osd_info()->set_font_text(osd_json["font_text"].asString());
			std::unordered_map<std::string, std::string>::const_iterator const_iter =  g_global_video_channel[video_split_info.multicast_port()].find(video_split_info.multicast_ip());
			if (const_iter != g_global_video_channel[video_split_info.multicast_port()].end() && const_iter ->second != video_split_info.split_channel_id() )
			{
				WARNING_EX_LOG("[videosplit split_channel_id = %s]  multicat ip = %s, port = %u failed !!!", video_split_info.split_channel_id().c_str(), video_split_info.multicast_ip().c_str(), video_split_info.multicast_port());

			}
			 else
			{
				g_global_video_channel[video_split_info.multicast_port()][video_split_info.multicast_ip()] = video_split_info.split_channel_id();
			}
			if (!m_video_split_info_map.insert(std::make_pair(video_split_info.split_channel_id(), video_split_info)).second)
			{
				WARNING_EX_LOG("[videosplit split_channel_id = %s] insert video_split info map failed !!!", video_split_info.split_channel_id().c_str());
			}
			if (video_split_info.id() > m_video_split_index)
			{
				m_video_split_index = video_split_info.id() + 1;
			}
			else if (video_split_info.id() == m_video_split_index)
			{
				++m_video_split_index;
			}
			//if ()
		}

	}
	void cvideo_split_info_mgr::_parse_camera_group(const Json::Value& camera_group_json, VideoSplitInfo& video_split_info)
	{
		for (uint32 i = 0; i < camera_group_json.size(); ++i)
		{
			CameraGroup *camera_group_ptr =  video_split_info.add_camera_group();
			//CameraGroup camera_group;
			if (!camera_group_json[i].isMember("index") || !camera_group_json[i]["index"].isUInt64())
			{
				WARNING_EX_LOG("config  warr index !!!");
				continue;
			}
			if (!camera_group_json[i].isMember("camera_id") || !camera_group_json[i]["camera_id"].isUInt64())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			if (!camera_group_json[i].isMember("x") || !camera_group_json[i]["x"].isDouble())
			{
				WARNING_EX_LOG("config  warr x !!!");
				continue;
			}
			if (!camera_group_json[i].isMember("y") || !camera_group_json[i]["y"].isDouble())
			{
				WARNING_EX_LOG("config  warr y !!!");
				continue;
			}
			if (!camera_group_json[i].isMember("w") || !camera_group_json[i]["w"].isDouble())
			{
				WARNING_EX_LOG("config  warr w !!!");
				continue;
			}
			if (!camera_group_json[i].isMember("h") || !camera_group_json[i]["h"].isDouble())
			{
				WARNING_EX_LOG("config  warr h !!!");
				continue;
			}
			camera_group_ptr->set_index(camera_group_json[i]["index"].asUInt64());
			camera_group_ptr->set_camera_id(camera_group_json[i]["camera_id"].asUInt64());
			camera_group_ptr->set_x(camera_group_json[i]["x"].asDouble());
			camera_group_ptr->set_y(camera_group_json[i]["y"].asDouble());
			camera_group_ptr->set_w(camera_group_json[i]["w"].asDouble());
			camera_group_ptr->set_h(camera_group_json[i]["h"].asDouble());
		 
		}
	}
	void cvideo_split_info_mgr::_sync_save_video_split_data()
	{
		Json::Value value;
		// 20240928 TODO@chensong 先清除所有设备信息然后再添加
		g_vms_device_mgr.clear_all_device_infos();
		for (VIDEO_SPLIT_INFO_MAP::const_iterator iter = m_video_split_info_map.begin();
			iter != m_video_split_info_map.end(); ++iter)
		{
			g_vms_device_mgr.update_device_info(iter->second.id(), iter->second.split_channel_id(), iter->second.split_channel_name(), 0);
			Json::Value video_split_info;
			video_split_info["id"] = static_cast<uint32>(iter->second.id());
			video_split_info["split_channel_name"] = iter->second.split_channel_name();
			video_split_info["split_channel_id"] = iter->second.split_channel_id();
			video_split_info["multicast_ip"] = iter->second.multicast_ip();
			video_split_info["multicast_port"] = iter->second.multicast_port();
			video_split_info["split_method"] = iter->second.split_method();
		//	video_split_info["split_type"] = static_cast<uint32>(iter->second.split_type());
			video_split_info["lock_1080p"] = iter->second.lock_1080p();
			video_split_info["overlay"] = iter->second.overlay();
			video_split_info["out_video_width"] = iter->second.out_video_width();
			video_split_info["out_video_height"] = iter->second.out_video_height();
			video_split_info["camera_group_size"] = iter->second.camera_groups_size();
			for (size_t i = 0; i < iter->second.camera_group_size(); ++i)
			{
				Json::Value Camera_group_json;
				Camera_group_json["index"] = iter->second.camera_group(i).index();
				Camera_group_json["camera_id"] = iter->second.camera_group(i).camera_id();
				Camera_group_json["x"] = iter->second.camera_group(i).x();
				Camera_group_json["y"] = iter->second.camera_group(i).y();
				Camera_group_json["w"] = iter->second.camera_group(i).w();
				Camera_group_json["h"] = iter->second.camera_group(i).h();
				video_split_info["camera_group"].append(Camera_group_json);
			}


			

			Json::Value osd_info;
			osd_info["x"] = iter->second.osd_info().x() == 0 ? 0.0000: iter->second.osd_info().x();
			osd_info["y"] = iter->second.osd_info().y() == 0 ? 0.0000 : iter->second.osd_info().y();
			osd_info["font_size"] = iter->second.osd_info().font_size();
			osd_info["font_text"] = iter->second.osd_info().font_text();

			video_split_info["osd_info"] = osd_info;
			//camera[""]
			value.append(video_split_info);
		}
		Json::StyledWriter swriter;
		std::string str = swriter.write(value);

		if (str.length() < 1)
		{
			WARNING_EX_LOG("not find data json --> [%s]", str.c_str());
			return;
		}
		//TODO@chensong 2024-02-06创建对应目录
		boost::filesystem::create_directories(g_cfg.get_string(ECI_DataPath));
		std::string video_split_json_name = g_cfg.get_string(ECI_DataPath) + "/" + g_video_split_json_name;
		FILE* write_file_ptr = ::fopen(video_split_json_name.c_str(), "wb+");
		if (!write_file_ptr)
		{

			WARNING_EX_LOG("not open data file [%s] failed !!!", video_split_json_name.c_str());
			return;
		}

		::fwrite(str.c_str(), 1, str.length(), write_file_ptr);
		::fflush(write_file_ptr);
		::fclose(write_file_ptr);
		write_file_ptr = NULL;

	}
}