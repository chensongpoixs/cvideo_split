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
//#include "croom.h"
#include <vector>
namespace chen {
	BEGIN_PROXY_MAP(cweb_http_api) 

		PROXY_WORKER_METHOD1(cresult_app_info, create_render_app, const create_render_app_struct&)
		//PROXY_WORKER_METHOD1(cresult_app_info,	update_render_app, const update_render_app_struct&)
		//PROXY_WORKER_METHOD1(uint32_t, delete_render_app, const std::string&)
		//PROXY_WORKER_METHOD1(uint32_t,  cmd_render_app, const cmd_render_app_struct& )
		//virtual uint32_t  kick_room_username(const std::string& room_name, const std::string & user_name)
		//PROXY_WORKER_METHOD2(uint32_t, kick_room_username, const std::string &, const std::string &)
			//PROXY_WORKER_METHOD2(uint32_t, add_while_room_username, const std::string &, const std::string &)
			//PROXY_WORKER_METHOD2(uint32_t, delete_while_room_username, const std::string &, const std::string &)
	END_PROXY_MAP()


		
}
extern chen::cweb_http_api_proxy g_web_http_api_proxy;
#endif //_C_WEB_API_PROXY_H_