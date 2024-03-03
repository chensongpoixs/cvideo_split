
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
		virtual   cresult_app_info create_render_app(const create_render_app_struct& msg) = 0;
		virtual   cresult_app_info update_render_app(const update_render_app_struct& msg) = 0;
		virtual   uint32_t delete_render_app(const std::string& app_id) = 0;
		virtual   uint32_t cmd_render_app(const cmd_render_app_struct & msg)  = 0;
		//virtual std::vector< croom_info>   get_all_room() = 0;
		//virtual std::vector< chen::cuser_info>   get_room_info(const std::string& room_name ) = 0;
		//
		//virtual uint32_t  kick_room_username(const std::string& room_name, const std::string & user_name) = 0;
		//virtual uint32_t  add_while_room_username(const std::string& room_name, const std::string & user_name) = 0;
		//virtual uint32_t  delete_while_room_username(const std::string& room_name, const std::string & user_name) = 0;
		virtual ~cweb_http_api_interface() {}
	};
}

#endif // _C_WEB_API_INTERFACE_H_
