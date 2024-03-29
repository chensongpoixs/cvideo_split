﻿/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#ifndef  _C_VIDEO_SPLIT_MGR_H_
#define  _C_VIDEO_SPLIT_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include <unordered_map>
#include "cvideo_split.h"
namespace chen {

	class cvideo_split_mgr : public cnoncopytable
	{
	private:
		typedef    std::unordered_map<std::string, cvideo_splist*>		VIDEO_SPLIST_MAP;
	public:
		explicit cvideo_split_mgr()
			: m_video_split_map()
			, m_gpu_use(){}
		virtual ~cvideo_split_mgr()
		{}

	public:
		bool init();
		void udpate(uint32 uDateTime);
		void destroy();
	public:
		uint32 handler_web_cmd_video_split(const std::string & channel_name/*uint32 id*/, uint32 cmd);
		
	public:

	private:
		uint32 use_gpu_index() const ;
	private:
		VIDEO_SPLIST_MAP					m_video_split_map;
		std::vector<uint32>				    m_gpu_use;
	};
	extern cvideo_split_mgr		g_video_split_mgr;
}
#endif // _C_VIDEO_SPLIT_MGR_H_
