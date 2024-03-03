/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#ifndef  _C_CAMERA_DECODER_MGR_H_
#define  _C_CAMERA_DECODER_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include "cdecoder.h"
#include <unordered_map>
namespace chen {
	class ccamera_decoder_mgr
	{
	private:
		typedef std::unordered_map<uint32, cdecoder*>         ALL_CAMERA_DECODER_MAP;  // camera_id -> websocker session id
		
		 
	public:
		explicit ccamera_decoder_mgr() 
		: m_all_camera_decoder_map(){}
		virtual ~ccamera_decoder_mgr() {}



	public:
		bool init();
		void update();
		void destroy();
	public:

		void websocket_add_camera_websocket_id(uint32 camera_id, uint32 session_id);
		void websocket_delete_camera_websocket_id(uint32 camera_id, uint32 session_id);

	public:

		uint32 handler_web_cmd_camera_id(uint32 camera_id, uint32 cmd);




	private:
		ALL_CAMERA_DECODER_MAP				m_all_camera_decoder_map;
		 
	};
}

#endif //_C_CAMERA_DECODER_MGR_H_