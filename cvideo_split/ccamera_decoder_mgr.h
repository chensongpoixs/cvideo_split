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
namespace chen {
	class ccamera_decoder_mgr
	{
	public:
		explicit ccamera_decoder_mgr() {}
		virtual ~ccamera_decoder_mgr() {}
	};
}

#endif //_C_CAMERA_DECODER_MGR_H_