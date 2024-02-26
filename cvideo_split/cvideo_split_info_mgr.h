/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	video_split mgr 

*********************************************************************/

#ifndef  _C_VIDEO_SPLIT_INFO_MGR_H_
#define  _C_VIDEO_SPLIT_INFO_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include "ccamera_info_mgr.h"
#include <unordered_map>
namespace chen {


	class cvideo_split_info_mgr
	{
	private:
		typedef std::unordered_map<std::string, VideoSplitInfo>			VIDEO_SPLIT_INFO_MAP;
	public:
		explicit cvideo_split_info_mgr()
		: m_data_type(EDataNone)
		, m_video_split_info_map()
		, m_video_split_index(0){}
		virtual ~cvideo_split_info_mgr(){}

	public:
		bool init();
		void update(uint32 uDelta);
		void destroy();

	public:

		cresult_add_video_split handler_web_add_video_split(  VideoSplitInfo& video_split_info);
		cresult_get_video_split handler_web_get_video_split(const std::string& channel_id) const;
		cresult_video_split_list handler_web_video_split_list(uint32 page, uint32 page_size);
		uint32					handler_web_delete_video_split(const std::string & channel_id/*uint32 id*/);
		uint32					handler_web_modify_video_split(const std::string& channel_id, const std::string& txt, uint32 fontsize, double x, double y);
	public:
		const  VideoSplitInfo* get_video_split_info(const std::string& channel_id/*uint32 id*/) const ;
	public:
		void _load_video_split_config();





		void _parse_json_data(const std::string & json_data);


		void _parse_camera_group(const Json::Value & camera_group, VideoSplitInfo & video_split_info);

		void _sync_save_video_split_data();
	private:
		EDataType							m_data_type;
		VIDEO_SPLIT_INFO_MAP						m_video_split_info_map;
		uint32								m_video_split_index;
	};
	extern cvideo_split_info_mgr g_video_split_info_mgr;
}
#endif // _C_VIDEO_SPLIT_INFO_MGR_H_