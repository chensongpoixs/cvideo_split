/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#include "cvideo_split_mgr.h"
#include "chttp_code.h"
#include "cvideo_split_info_mgr.h"
#include "cvideo_split_server.h"
namespace chen {
	cvideo_split_mgr		g_video_split_mgr;
	bool cvideo_split_mgr::init()
	{
		for (uint32 i = 0; i < g_gpu_index.size(); ++i)
		{
			m_gpu_use.push_back(0);
		}
		return true;
	}
	void cvideo_split_mgr::udpate(uint32 uDateTime)
	{
		for (VIDEO_SPLIST_MAP::iterator iter = m_video_split_map.begin(); iter != m_video_split_map.end(); ++iter)
		{
			if (iter->second)
			{
				iter->second->update(uDateTime);
			}
		}
	}
	void cvideo_split_mgr::destroy()
	{
		for (VIDEO_SPLIST_MAP::iterator iter = m_video_split_map.begin(); iter != m_video_split_map.end(); ++iter)
		{
			if (iter->second)
			{
				iter->second->destroy();
				delete iter->second;
			}
		}
		m_video_split_map.clear();
	}
	uint32 cvideo_split_mgr::handler_web_cmd_video_split(const std::string& channel_name/*uint32 id*/, uint32 cmd)
	{
		VIDEO_SPLIST_MAP::iterator iter = m_video_split_map.find(channel_name);
		if (cmd != 0)
		{
			
			if (iter == m_video_split_map.end())
			{ 
				return EWebVideoSplitNotStart;
			}
			uint32 use_gpu_index = iter->second->get_use_gpu_index();
			iter->second->destroy();
			cvideo_splist* video_split_ptr = iter->second;
			size_t size = m_video_split_map.erase(channel_name);
			if (size > 0 && video_split_ptr)
			{
				if (use_gpu_index < m_gpu_use.size())
				{
					--m_gpu_use[use_gpu_index];
				}
				else
				{
					WARNING_EX_LOG("[gpu size = %u][use gpu index = %u]", m_gpu_use.size(), use_gpu_index);
				}
				//iter->second->destroy();
				delete video_split_ptr;
				VideoSplitInfo* video_split_info_ptr = g_video_split_info_mgr.get_video_split_info(channel_name);
				if (!video_split_info_ptr)
				{
					WARNING_EX_LOG("not find channel name = %s", channel_name.c_str());
				}
				else
				{
					video_split_info_ptr->set_status(0);
				}
				return EWebSuccess;
			}
			WARNING_EX_LOG("delete video split [channel_name = %s] failed !!!", channel_name.c_str());
			return EWebWait;

		}
		if (iter != m_video_split_map.end())
		{
			return EWebVideoSplitStarting;
		}
		 
		// TODO@chensong 2023-02-20 获取拼接信息
		 VideoSplitInfo* video_split_info_ptr =  g_video_split_info_mgr.get_video_split_info(channel_name);
		if (!video_split_info_ptr)
		{
			return EWebNotFindVideoSplitId;
		}

		cvideo_splist* video_split_ptr = new cvideo_splist();
		if (!video_split_ptr)
		{
			WARNING_EX_LOG("alloc video split channel_name =   (%s) failed !!!", channel_name.c_str());
			return EWebWait;
		}
		
		uint32 gpu_index = use_gpu_index();   
		 

		if (!video_split_ptr->init(gpu_index, video_split_info_ptr))
		{
			video_split_ptr->destroy();
			delete video_split_ptr;
			video_split_ptr = NULL;
			WARNING_EX_LOG("init video split  channel_name =  (%s) failed !!!", channel_name.c_str());
			return EWebWait;
		}
		std::pair<VIDEO_SPLIST_MAP::iterator, bool>  pi = m_video_split_map.insert(std::make_pair(channel_name, video_split_ptr));
		if (!pi.second)
		{
			video_split_ptr->destroy();
			delete video_split_ptr;
			video_split_ptr = NULL;
			WARNING_EX_LOG("insert video split map channel_name = (%s) failed !!!", channel_name.c_str());
			return EWebWait;
		}
		if (m_gpu_use.size() > gpu_index)
		{
			++m_gpu_use[gpu_index];
		}
		video_split_info_ptr->set_status(1);
		return EWebSuccess;
	}
	uint32 cvideo_split_mgr::use_gpu_index() const
	{
		uint32 count = 3567587328;
		uint32 index = 0;
		for (uint32 i = 0; i < m_gpu_use.size(); ++i)
		{
			if (count > m_gpu_use[i])
			{
				count = m_gpu_use[i];
				index = i;
			}
		}
		if (count < g_cfg.get_uint32(ECI_UseGpuMaxCount))
		{

			//TODO@chensong 2024-03-05 有可能显卡内存不够了ffmpeg中会崩溃
			return index;
		}
		return 0;
	}
}