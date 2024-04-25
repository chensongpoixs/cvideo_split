/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/
#include "ccamera_info_mgr.h"
#include "ccfg.h"
#include <json/json.h>
#include "clog.h"
#include "chttp_code.h"
#include <json/json.h>
#include "cdecode.h"
#include "cudp.h"
namespace chen {



	ccamera_info_mgr g_camera_info_mgr;

	static const char* camera_json_name = "camera_list.json";

	bool ccamera_info_mgr::init()
	{
		_load_camera_config();
		m_check_camera_timestamp = 0;
		update(0);
		//m_check_camera_timestamp = ::time(NULL) + g_cfg.get_uint32(ECI_CheckCameraStatus);
		m_stoped = false;
		m_check_camera_status_thread = std::thread(&ccamera_info_mgr::_pthread_check_camera_status, this);
		//m_check_camera_timestamp = ::time(NULL);
		return true;
	}
	void ccamera_info_mgr::update(uint32 uDateTime)
	{
		 
		if (::time(NULL) >  (g_cfg.get_uint32(ECI_CheckCameraStatus) + m_check_camera_timestamp))
		{
			{
				clock_guard lock(m_checking_camera_lock);
				if (m_checking_camera_info_status_list.empty())
				{
					for (std::pair<const uint32, CameraInfo>& pi : m_camera_info_map)
					{
						m_checking_camera_info_status_list.push_back(pi.second);
					}
				}
				
			}


			m_check_camera_timestamp = ::time(NULL);
		}
		{

			clock_guard  lock(m_checkend_camera_lock);
			for (size_t i = 0; i < m_checkend_camera_info_status_list.size(); ++i)
			{ 
				CAMERA_INFO_MAP::iterator iter =  m_camera_info_map.find(m_checkend_camera_info_status_list[i].camera_id());
				if (iter != m_camera_info_map.end())
				{
					iter->second.set_state(m_checkend_camera_info_status_list[i].state());
				}
				else
				{
					WARNING_EX_LOG("not find [camera id = %u][%s:%u]", m_checkend_camera_info_status_list[i].camera_id(), m_checkend_camera_info_status_list[i].ip().c_str(), m_checkend_camera_info_status_list[i].port());
				}
			}
			m_checkend_camera_info_status_list.clear();
		}
		if (m_data_type != EDataNone)
		{
			_write_all_camera_config();
			m_data_type = EDataNone;
		}
	}
	void ccamera_info_mgr::destroy()
	{
		m_stoped = true;
		if (m_check_camera_status_thread.joinable())
		{
			m_check_camera_status_thread.join();
		}
		if (m_data_type != EDataNone)
		{
			_write_all_camera_config();
			m_data_type = EDataNone;
		}
		m_camera_info_map.clear();

		m_checkend_camera_info_status_list.clear();
		m_checking_camera_info_status_list.clear();
	}
	cresult_add_camera_info ccamera_info_mgr::handler_add_camera_infos(const AddCameraInfos& msg)
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
				{
						clock_guard lock(m_checking_camera_lock);
						m_checking_camera_info_status_list.push_back(*camera_info_ptr);
				}
				//camera_info_ptr->set_camera_id();
			}
			m_data_type = EDataLoad;
		}
		return result;
	}


	cresult_add_camera_info ccamera_info_mgr::handler_modify_camera_infos(const AddCameraInfos& msg)
	{
		cresult_add_camera_info result;
		if (msg.camera_infos_size() > 0)
		{
			for (size_t i = 0; i < msg.camera_infos_size(); ++i)
			{
				CameraInfo* camera_info_ptr = result.camera_infos.add_camera_infos();
				if (!camera_info_ptr)
				{
					WARNING_EX_LOG("protobut alloc failed !!! ");
					continue;
				}
				*camera_info_ptr = msg.camera_infos(i);

				CAMERA_INFO_MAP::iterator iter =  m_camera_info_map.find(camera_info_ptr->camera_id());
				if (iter != m_camera_info_map.end())
				{
					iter->second.set_address(camera_info_ptr->address());
					iter->second.set_ip(camera_info_ptr->ip());
					iter->second.set_camera_name(camera_info_ptr->camera_name());
					iter->second.set_port(camera_info_ptr->port());
					{
						clock_guard lock(m_checking_camera_lock);
						m_checking_camera_info_status_list.push_back(*camera_info_ptr);
					}
				}
				else
				{
					result.result = EWebModifyNotFindCameraId;
					WARNING_EX_LOG("not modify failed !!! (cmaera_id = %u)", camera_info_ptr->camera_id());
				}
				/*camera_info_ptr->set_camera_id(m_camera_index);
				if (!m_camera_info_map.insert(std::make_pair(m_camera_index, *camera_info_ptr)).second)
				{
					WARNING_EX_LOG("web insert camera [index = %u] failed !!!", m_camera_index);
					continue;
				}

				m_camera_index++;*/
				
				//camera_info_ptr->set_camera_id();
			}
			m_data_type = EDataLoad;
		}
		else
		{
		//	result.result = 3;
			WARNING_EX_LOG("not find modify camera info data failed !!!");
		}
		return result;
	}


	cresult_camera_list ccamera_info_mgr::handler_camera_list(uint32 page, uint32 page_size)
	{
		cresult_camera_list result;

		


		result.page_info.set_total_pages((m_camera_info_map.size() / page_size) + ((m_camera_info_map.size() % page_size)> 0? 1: 0));
		result.page_info.set_total_elements(m_camera_info_map.size());
		result.page_info.set_page_number(page);
		int32 show_page_size = 0;
		int32 start_index =  (page_size * (page));
		if (m_camera_info_map.size() >= start_index)
		{
			show_page_size = (m_camera_info_map.size() - start_index)> page_size ? page_size : (m_camera_info_map.size() - start_index);
			result.page_info.set_page_size(show_page_size);
			//CAMERA_INFO_MAP::const_iterator iter = m_camera_info_map.find(camera_id);
			for (CAMERA_INFO_MAP::const_iterator iter = m_camera_info_map.begin();
				iter != m_camera_info_map.end(); ++iter)
			{
				if (start_index <= 0)
				{
					if (show_page_size <= 0)
					{
						return result;
					}
					CameraInfo* camera_info_ptr = result.camera_infos.add_camera_infos();
					if (!camera_info_ptr)
					{
						WARNING_EX_LOG("protobut alloc failed !!! ");
						continue;
					}
					*camera_info_ptr = iter->second;
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

	uint32			ccamera_info_mgr::handler_delete_camera(uint32 camera_id)
	{
		CAMERA_INFO_MAP::iterator iter =  m_camera_info_map.find(camera_id);
		if (iter != m_camera_info_map.end())
		{
			 m_camera_info_map.erase(iter);
			 NORMAL_EX_LOG("delete [camera_id = %u]", camera_id);
			_sync_save();
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
		WARNING_EX_LOG("not find [camera_id = %u]", camera_id);
		return EWebNotFindCameraId;
	}
	const CameraInfo* ccamera_info_mgr::get_camera_info(uint32 camera_id) const
	{
		CAMERA_INFO_MAP::const_iterator const_iterator =  m_camera_info_map.find(camera_id);
		if (const_iterator != m_camera_info_map.end())
		{
			return &const_iterator->second;
		}
		return NULL;
	}
	CameraInfo* ccamera_info_mgr::get_camera_info(uint32 camera_id)  
	{
		CAMERA_INFO_MAP::iterator iterator = m_camera_info_map.find(camera_id);
		if (iterator != m_camera_info_map.end())
		{
			return &iterator->second;
		}
		return NULL;
	}
	void ccamera_info_mgr::_load_camera_config()
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
	void ccamera_info_mgr::_write_all_camera_config()
	{
		Json::Value value;
		 
		for (CAMERA_INFO_MAP::const_iterator iter = m_camera_info_map.begin();
			iter != m_camera_info_map.end(); ++iter)
		{
			Json::Value camera;
			camera["camera_id"] = iter->second.camera_id();
			camera["ip"] = iter->second.ip();
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
	void ccamera_info_mgr::_sync_save()
	{
		m_data_type = EDataLoad;
	}
	void ccamera_info_mgr::_parse_json_data(const std::string& camera)
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
			if (!data[i].isMember("ip") || !data[i]["ip"].asCString())
			{
				WARNING_EX_LOG("config  warr ip !!!");
				continue;
			}
			if (!data[i].isMember("address") || !data[i]["address"].isString())
			{
				WARNING_EX_LOG("config  warr address !!!");
				continue;
			}
			if (!data[i].isMember("camera_name") || !data[i]["camera_name"].isString())
			{
				WARNING_EX_LOG("config  warr camera_name !!!");
				continue;
			}
			if (!data[i].isMember("port") || !data[i]["port"].isUInt())
			{
				WARNING_EX_LOG("config  warr port !!!");
				continue;
			}
			if (!data[i].isMember("url") || !data[i]["url"].isString())
			{
				WARNING_EX_LOG("config  warr url !!!");
				continue;
			}
			camera_info.set_ip(data[i]["ip"].asString());
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
	void ccamera_info_mgr::_pthread_check_camera_status()
	{
		CameraInfo camera_info;
		bool  camera = false;
		uint32  second = 0;
		while (!m_stoped)
		{
			{
				clock_guard lock(m_checking_camera_lock);
				if (m_checking_camera_info_status_list.size() > 0)
				{
					camera = true;
					camera_info = m_checking_camera_info_status_list.front();
					m_checking_camera_info_status_list.pop_front();
				}
			}
			if (camera && !m_stoped)
			{
				/*cdecode decode;
				std::string url = "udp://@" + camera_info.address() + ":" + std::to_string(camera_info.port());
				bool open = decode.init(0, url.c_str(), 10);
				decode.destroy();
				if (!open)
				{
					WARNING_EX_LOG("not open camera [%s ]", url.c_str());
				}
				camera_info.set_state(open == true ? ECameraRuning : ECameraNone);
				{
					clock_guard lock(m_checkend_camera_lock);
					m_checkend_camera_info_status_list.push_back(camera_info);
				}
				camera = false;*/
				//SYSTEM_LOG("start [%s:%u]", camera_info.address().c_str(), camera_info.port());
				bool open =   udp_util::test_udp_connect(camera_info.ip().c_str(), camera_info.address().c_str(), camera_info.port());
				if (!open)
				{
					WARNING_EX_LOG("not open camera[address = %s] [%s:%u ]", camera_info.ip().c_str(),  camera_info.address().c_str(), camera_info.port());
				}
				//SYSTEM_LOG("end [%s:%u]", camera_info.address().c_str(), camera_info.port());
				camera_info.set_state(open == true ? ECameraRuning : ECameraNone);
				{
					clock_guard lock(m_checkend_camera_lock);
					m_checkend_camera_info_status_list.push_back(camera_info);
				}
				camera = false;

			}
			if (!m_stoped)
			{
				clock_guard lock(m_checkend_camera_lock);
				if (m_checking_camera_info_status_list.empty())
				{
					second  = 100;
				  }
				else
				{
					second  = 0;
				}
			}
			if (second > 0 && !m_stoped)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(second));
				second = 0;
			}





		}
	}
}