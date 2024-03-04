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
			iter->second->destroy();
			size_t size = m_video_split_map.erase(channel_name);
			if (size > 0)
			{
				//iter->second->destroy();
				delete iter->second;
				VideoSplitInfo* video_split_info_ptr = g_video_split_info_mgr.get_video_split_info(channel_name);
				video_split_info_ptr->set_status(0);
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
		
		uint32 gpu_index = 0;   
		if (g_gpu_index.size() > 1)
		{
			gpu_index = m_video_split_map.size() % g_gpu_index.size();
		}

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
		video_split_info_ptr->set_status(1);
		return EWebSuccess;
	}
}