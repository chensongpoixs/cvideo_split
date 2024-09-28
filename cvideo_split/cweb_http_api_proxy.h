/********************************************************************
created:	2021-11-24

author:		chensong

purpose:	Location

*********************************************************************/

#ifndef _C_WEB_HTTP_API_PROXY_H_
#define _C_WEB_HTTP_API_PROXY_H_

#include <memory>
#include <string>
#include <utility>
#include "cweb_http_api_interface.h"
#include "cproxy.h"
#include "cweb_http_api_mgr.h"
#include "cweb_http_api_proxy.h"
#include "chttp_queue_mgr.h"
#include "VideoSplit.pb.h"
//#include "croom.h"
#include <vector>
namespace chen {
	BEGIN_PROXY_MAP(cweb_http_api)

		PROXY_WORKER_METHOD1(cresult_add_camera_info, add_camera_infos, const AddCameraInfos&)
		PROXY_WORKER_METHOD1(cresult_add_camera_info, modify_camera_infos, const AddCameraInfos&)
		PROXY_WORKER_METHOD2(cresult_camera_list, camera_list, uint32, uint32)
		PROXY_WORKER_METHOD1(uint32, delete_camera, uint32)


		PROXY_WORKER_METHOD1(cresult_add_video_split, add_video_split, const VideoSplitInfo&)
		PROXY_WORKER_METHOD1(cresult_get_video_split, get_video_split, const std::string&)
		PROXY_WORKER_METHOD2(cresult_video_split_list, video_split_list, uint32, uint32)
		PROXY_WORKER_METHOD1(uint32, delete_video_split, const std::string&)
		PROXY_WORKER_METHOD1(cresult_video_split_osd, modify_video_split, const VideoSplitOsd&);



		PROXY_WORKER_METHOD2(uint32, cmd_video_split, const std::string&, uint32)

		
		PROXY_WORKER_METHOD0(cresult_vms_server_config, get_vms_server_config );

		PROXY_WORKER_METHOD1(cresult_vms_server_config, modify_vms_server_config, const cvms_server_config&);
		PROXY_WORKER_METHOD1(uint32, cmd_vms_server, uint32)


	END_PROXY_MAP()


		
}
extern chen::cweb_http_api_proxy g_web_http_api_proxy;
#endif //_C_WEB_API_PROXY_H_