/********************************************************************
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
		typedef    std::unordered_map<uint32, cvideo_splist*>		VIDEO_SPLIST_MAP;
	public:
		explicit cvideo_split_mgr()
			: m_video_split_map(){}
		virtual ~cvideo_split_mgr()
		{}

	public:
		bool init();
		void udpate(uint32 uDateTime);
		void destroy();
	public:
		uint32 handler_web_cmd_video_split(uint32 id, uint32 cmd);
	public:


	private:
		VIDEO_SPLIST_MAP					m_video_split_map;
	};
	extern cvideo_split_mgr		g_video_split_mgr;
}
#endif // _C_VIDEO_SPLIT_MGR_H_
