/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr 

*********************************************************************/

#ifndef  _C_CAMERA_MGR_H_
#define  _C_CAMERA_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
namespace chen {

	enum EDataType
	{
		EDataNone = 0,
		EDataLoad ,
	};

	class ccamera_mgr : public cnoncopytable
	{
	private:
		typedef	std::map<uint32, CameraInfo>       CAMERA_INFO_MAP;
	public:
		explicit ccamera_mgr()
			: m_data_type(EDataNone)
		, m_camera_info_map()
		, m_camera_index(0){}
		virtual ~ccamera_mgr() {}
	public:
		//加载配置文件
		bool init();
		void update(uint32 uDateTime);
		void destroy();


	public:

		cresult_add_camera_info handler_add_camera_infos(const AddCameraInfos& msg);
		cresult_camera_list handler_camera_list(uint32 page, uint32 page_size);
		uint32			handler_delete_camera(uint32 camera_id);

	private:
		void _load_camera_config();

		void _write_all_camera_config();


	private:
		void _parse_json_data(const std::string & data);
	private:

		EDataType						m_data_type;
		CAMERA_INFO_MAP					m_camera_info_map;
		uint32							m_camera_index;
	};
	extern ccamera_mgr g_camera_mgr;
}
#endif // 