/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/
#include "ccamera_mgr.h"
#include "ccfg.h"
#include <json/json.h>
#include "clog.h"
#include <json/json.h>
namespace chen {



	ccamera_mgr g_camera_mgr;

	static const char* camera_json_name = "camera_list.json";

	bool ccamera_mgr::init()
	{
		_load_camera_config();
		return true;
	}
	void ccamera_mgr::update(uint32 uDateTime)
	{
		if (m_data_type != EDataNone)
		{
			_write_all_camera_config();
			m_data_type = EDataNone;
		}
	}
	void ccamera_mgr::destroy()
	{
		if (m_data_type != EDataNone)
		{
			_write_all_camera_config();
			m_data_type = EDataNone;
		}
		m_camera_info_map.clear();
	}
	cresult_add_camera_info ccamera_mgr::handler_add_camera_infos(const AddCameraInfos& msg)
	{
		cresult_add_camera_info result;
		if (msg.camera_infos_size() > 0)
		{
			for (size_t i = 0; i < msg.camera_infos_size(); ++i)
			{
				CameraInfo * camera_info_ptr = result.camera_infos.add_camera_infos();
				if (!camera_info_ptr)
				{
					WARNING_EX_LOG("protobut alloc failed !!! ");
					continue;
				}
				*camera_info_ptr = msg.camera_infos(i);
				camera_info_ptr->set_camera_id(m_camera_index );
				if (!m_camera_info_map.insert(std::make_pair(m_camera_index, *camera_info_ptr)).second)
				{
					WARNING_EX_LOG("web insert camera [index = %u] failed !!!", m_camera_index);
					continue;
				}

					m_camera_index++;
				//camera_info_ptr->set_camera_id();
			}
			m_data_type = EDataLoad;
		}
		return result;
	}
	void ccamera_mgr::_load_camera_config()
	{
		std::string camera_name = g_cfg.get_string(ECI_DataPath) + "/" + camera_json_name;
		FILE* read_file_ptr = ::fopen(camera_name.c_str(), "rb");
		if (!read_file_ptr)
		{
		
			WARNING_EX_LOG("not open data file [%s] failed !!!", camera_name.c_str());
			return;
		}

		// 数据的大小
		// 移动到文件末尾
		::fseek(read_file_ptr, 0, SEEK_END);
		// 获取当前位置，即文件大小
		uint32 size = ::ftell(read_file_ptr);
		::fseek(read_file_ptr, 0, SEEK_SET);


		std::string camera_data;
		camera_data.resize(size +1);
		size_t read_size = ::fread((void *)camera_data.data(), 1, size, read_file_ptr);


		if (read_file_ptr)
		{
			::fclose(read_file_ptr);
			read_file_ptr = NULL;
		}
		/// parse json 
		_parse_json_data(camera_data);
	}
	void ccamera_mgr::_write_all_camera_config()
	{
		Json::Value value;
		 
		for (CAMERA_INFO_MAP::const_iterator iter = m_camera_info_map.begin();
			iter != m_camera_info_map.end(); ++iter)
		{
			Json::Value camera;
			camera["camera_id"] = iter->second.camera_id();
			camera["camera_name"] = iter->second.camera_name();
			camera["address"] = iter->second.address();
			camera["port"] = iter->second.port();
			camera["url"] = iter->second.url();
			//camera[""]
			value.append(camera);
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
		std::string camera_name = g_cfg.get_string(ECI_DataPath) + "/" + camera_json_name;
		FILE* write_file_ptr = ::fopen(camera_name.c_str(), "wb+");
		if (!write_file_ptr)
		{

			WARNING_EX_LOG("not open data file [%s] failed !!!", camera_name.c_str());
			return;
		}

		::fwrite(str.c_str(), 1, str.length(), write_file_ptr);
		::fflush(write_file_ptr);
		::fclose(write_file_ptr);
		write_file_ptr = NULL;

		//printf("[camera_list = %s]\n", str.c_str());
	}
	void ccamera_mgr::_parse_json_data(const std::string& camera)
	{
		Json::Reader reader;
		Json::Value data;
		 
		if (!reader.parse((const char*)camera.c_str(), (const char*)(camera.c_str() + camera.size()), data))
		{
			WARNING_EX_LOG("parse  [camera_list = %s] failed !!!", camera.c_str());
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
			if (!data[i].isMember("camera_id") || !data[i]["camera_id"].isUInt64())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			if (!data[i].isMember("address") || !data[i]["address"].isString())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			if (!data[i].isMember("camera_name") || !data[i]["camera_name"].isString())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			if (!data[i].isMember("port") || !data[i]["port"].isUInt())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			if (!data[i].isMember("url") || !data[i]["url"].isString())
			{
				WARNING_EX_LOG("config  warr camera_id !!!");
				continue;
			}
			camera_info.set_camera_id(data[i]["camera_id"].asUInt64());
			camera_info.set_address(data[i]["address"].asString());
			camera_info.set_camera_name(data[i]["camera_name"].asString());
			camera_info.set_port(data[i]["port"].asUInt());
			camera_info.set_url(data[i]["url"].asString()); 
			if (!m_camera_info_map.insert(std::make_pair(camera_info.camera_id(), camera_info)).second)
			{
				WARNING_EX_LOG("[camera id = %u] insert camera info map failed !!!", camera_info.camera_id());
			}
			if (camera_info.camera_id() > m_camera_index)
			{
				m_camera_index = camera_info.camera_id() + 1;
			}
			else if (camera_info.camera_id() == m_camera_index)
			{
				++m_camera_index;
			}
			//if ()
		}

	}
}