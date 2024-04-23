/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#ifndef  _C_CAMERA_STATUS_MGR_H_
#define  _C_CAMERA_STATUS_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include <mutex>
namespace chen {





	struct ccmamera_status
	{
		uint32	id;
		std::string ip;
		uint32 port;

		time_t timestamp;
	};


	class ccamera_status_mgr
	{
	private:
		typedef  std::mutex						clock_type;
		typedef  std::lock_guard<clock_type>	clock_guard;
	public:
		explicit ccamera_status_mgr() {}
		virtual ~ccamera_status_mgr() {}
	public:
		bool init();
		void update(uint32 DataTime);
		void destroy();

	private:

	};


}

#endif // _C_CAMERA_STATUS_MGR_H_