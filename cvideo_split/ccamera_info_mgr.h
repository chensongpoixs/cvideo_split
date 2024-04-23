/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr 

*********************************************************************/

#ifndef  _C_CAMERA_INFO_MGR_H_
#define  _C_CAMERA_INFO_MGR_H_
#include "cnoncopytable.h"
#include "cweb_http_api_proxy.h"
#include "cweb_http_struct.h"
#include "cnet_type.h"
#include "VideoSplit.pb.h"
#include <map>
#include <mutex>

namespace chen {

	enum EDataType
	{
		EDataNone = 0,
		EDataLoad ,
	};

	class ccamera_info_mgr : public cnoncopytable
	{
	private:
		typedef	std::map<uint32, CameraInfo>       CAMERA_INFO_MAP;
		typedef std::mutex							clock_type;
		typedef std::lock_guard<clock_type>			clock_guard;
	public:
		explicit ccamera_info_mgr()
			: m_stoped(false)
			, m_data_type(EDataNone)
		, m_camera_info_map()
		, m_camera_index(0)
		, m_checking_camera_info_status_list()
		, m_checkend_camera_info_status_list()
		, m_check_camera_timestamp(::time(NULL)){}
		virtual ~ccamera_info_mgr() {}
	public:
		//加载配置文件
		bool init();
		void update(uint32 uDateTime);
		void destroy();


	public:

		cresult_add_camera_info handler_add_camera_infos(const AddCameraInfos& msg);
		cresult_camera_list handler_camera_list(uint32 page, uint32 page_size);
		uint32			handler_delete_camera(uint32 camera_id);


	public:
		const CameraInfo* get_camera_info(uint32 camera_id) const ;
		CameraInfo* get_camera_info(uint32 camera_id);
	private:
		void _load_camera_config();

		void _write_all_camera_config();

		void _sync_save();
	private:
		void _parse_json_data(const std::string & data);



		void _pthread_check_camera_status();
	private:
		bool							m_stoped;
		EDataType						m_data_type;
		CAMERA_INFO_MAP					m_camera_info_map;
		uint32							m_camera_index;


		std::thread						m_check_camera_status_thread;
		clock_type						m_checking_camera_lock;
		std::list< CameraInfo>			m_checking_camera_info_status_list;
		clock_type						m_checkend_camera_lock;
		std::vector< CameraInfo>			m_checkend_camera_info_status_list;
		time_t							m_check_camera_timestamp;
	};
	extern ccamera_info_mgr g_camera_info_mgr;
}
#endif // 