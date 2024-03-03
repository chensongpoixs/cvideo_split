/********************************************************************
created:	2024-02-06

author:		chensong

purpose:	camera mgr

*********************************************************************/

#include"ccamera_decoder_mgr.h"
#include "ccamera_info_mgr.h"
#include "clog.h"
namespace chen {
	bool ccamera_decoder_mgr::init()
	{
		return true;
	}
	void ccamera_decoder_mgr::update()
	{
	}
	void ccamera_decoder_mgr::destroy()
	{
		for (ALL_CAMERA_DECODER_MAP::iterator iter = m_all_camera_decoder_map.begin(); iter != m_all_camera_decoder_map.end();
			++iter)
		{
			if (iter->second)
			{
				iter->second->destroy();
				delete iter->second;
			}
		}
		m_all_camera_decoder_map.clear();
	}
	void ccamera_decoder_mgr::websocket_add_camera_websocket_id(uint32 camera_id, uint32 session_id)
	{
		ALL_CAMERA_DECODER_MAP::iterator iter = 	m_all_camera_decoder_map.find(camera_id);
		if (iter != m_all_camera_decoder_map.end())
		{
			iter->second->add_websocket_session(session_id);
			return;
		}
		  CameraInfo* camera_info_ptr = g_camera_info_mgr.get_camera_info(camera_id);
		  if (!camera_info_ptr)
		  {
			  WARNING_EX_LOG("not find camera_id =%u", camera_id);
			  return;
		  }
		  cdecoder* decoder_ptr = new cdecoder();
		  if (!decoder_ptr)
		  {
			  WARNING_EX_LOG("alloc decoder failed   (camera_id = %u)!!!", camera_id);
			  return;
		  }
		  if (!decoder_ptr->init(1, camera_info_ptr->url().c_str()))
		  {
			  WARNING_EX_LOG("not open url = [%s]", camera_info_ptr->url().c_str());
			  delete decoder_ptr;
			  decoder_ptr = NULL;
			  return;
		  }
		  if (m_all_camera_decoder_map.insert(std::make_pair(camera_id, decoder_ptr)).second)
		  {
			  WARNING_EX_LOG(" insert camera player (camera_id = %u) failed !!! url = [%s]", camera_id, camera_info_ptr->url().c_str());
			  decoder_ptr->destroy();
			  delete decoder_ptr;
			  decoder_ptr = NULL;
			  return;
		  }
		  decoder_ptr->add_websocket_session(session_id);
		  camera_info_ptr->set_state(ECameraRuning);

	}
	void ccamera_decoder_mgr::websocket_delete_camera_websocket_id(uint32 camera_id, uint32 session_id)
	{
		ALL_CAMERA_DECODER_MAP::iterator iter = m_all_camera_decoder_map.find(camera_id);
		if (iter != m_all_camera_decoder_map.end())
		{
			iter->second->delete_websocket_session(session_id);
			return;
		}
		WARNING_EX_LOG("not find camera_id = %u", camera_id);
	}
	uint32 ccamera_decoder_mgr::handler_web_cmd_camera_id(uint32 camera_id, uint32 cmd)
	{
		return uint32();
	}
}