
/********************************************************************
created:	2021-11-24

author:		chensong

purpose:	Location

*********************************************************************/

#ifndef _C_WEB_HTTP_API_INTERFACE_H_
#define _C_WEB_HTTP_API_INTERFACE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>
//#include "RenderCentralDefine.pb.h"
#include "cweb_http_struct.h"
//#include "croom.h"
//#include "croom_mgr.h"
namespace chen {
	class cweb_http_api_interface
	{
	public:

		//virtual	  uint32_t update_auth_info(uint64_t auth_id) = 0;
		virtual   cresult_add_camera_info add_camera_infos(const AddCameraInfos& msg) = 0;
		virtual   cresult_add_camera_info modify_camera_infos(const AddCameraInfos& msg) = 0;
		virtual   cresult_camera_list camera_list(uint32 page, uint32 page_size) = 0;
		virtual	  uint32			  delete_camera(uint32 camera_id) = 0;


		virtual	  cresult_add_video_split add_video_split( const VideoSplitInfo& msg) = 0;
		virtual   cresult_get_video_split get_video_split(const std::string& channel_id/*uint32 id*/) = 0;

		virtual  cresult_video_split_list video_split_list(uint32 page, uint32 page_size) = 0;
		virtual   uint32				delete_video_split(const std::string& channel_id/*uint32 id*/) = 0;

		virtual uint32	cmd_video_split(const std::string & channel_id/*uint32 id*/, uint32 cmd) = 0;


		virtual   cresult_video_split_osd			  modify_video_split(const VideoSplitOsd &video_osd) = 0;

		virtual   cresult_vms_server_config           modify_vms_server_config(const cvms_server_config & server_config) = 0;
		virtual   cresult_vms_server_config           get_vms_server_config() = 0;

		virtual   uint32								cmd_vms_server(uint32 cmd) = 0;

		virtual ~cweb_http_api_interface() {}
	};
}

#endif // _C_WEB_API_INTERFACE_H_
