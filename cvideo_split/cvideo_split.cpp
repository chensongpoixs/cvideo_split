/***********************************************************************************************
created: 		2024-01-26

author:			chensong

purpose:		camera

输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/
#include "cvideo_split.h"
#include "clog.h"
#include "ccfg.h"
#include "ctime_stamp.h"
#include "cvideo_split_mgr.h"
namespace chen {
	static				std::mutex   g_avfilter_lock;


	static	const int64				g_frame_cryle = 65535;
	/*
	
	def calculate_pts(frame_number, timebase, framerate, container_timebase):
	# 将时间基转换为秒
	timebase_in_seconds = timebase / 1000000.0
	# 计算每个编码帧的时间长度（秒）
	pts_per_frame = timebase_in_seconds / framerate
	# 将容器时间基转换为秒
	container_timebase_in_seconds = container_timebase / 1000000000.0
	# 计算PTS
	pts = int((frame_number * pts_per_frame) * (container_timebase_in_seconds / timebase_in_seconds))
	return pts

	# 示例使用
	timebase = 1000000  # 假设编码器时间基是微秒
	framerate = 25     # 25fps
	container_timebase = 1000000000  # 假设容器时间基是纳秒

	# 第一个视频帧的PTS
	print(calculate_pts(0, timebase, framerate, container_timebase))
	# 第二个视频帧的PTS
	print(calculate_pts(1, timebase, framerate, container_timebase))
	*/

	static uint64  global_calculate_pts(uint64 frame_number , uint32 frame_rate)
	{
		static const uint64 container_timebase = 1000000000;
		//# 将时间基转换为秒
		static const uint64 timebase_in_seconds = AV_TIME_BASE / 1000000.0;
			//# 计算每个编码帧的时间长度（秒）
		static const uint64 	pts_per_frame = timebase_in_seconds / frame_rate;
		//	# 将容器时间基转换为秒
		static const uint64	container_timebase_in_seconds = container_timebase / 1000000000.0;
		//	# 计算PTS
		uint64	pts = int((frame_number * pts_per_frame) * (container_timebase_in_seconds / timebase_in_seconds));
		return pts;
	}
	static std::string osd_text_str(const std::string& old)
	{
		std::string txt = old;
		while (txt.length() > 0 && txt[txt.length() - 1] == '\n')
		{
			txt = txt.substr(0, txt.length() - 1);
		}
		return txt;
	}


	EWebCode cvideo_splist::init(uint32 gpu_index, const VideoSplitInfo* video_split_info)
	{
		if (!video_split_info)
		{
			return EWebWait;
		}
		m_gpu_index = gpu_index;
		m_stoped = false;
		m_id = video_split_info->id();
		m_video_split_name = video_split_info->split_channel_name();
		m_video_split_channel = video_split_info->split_channel_id();
		m_multicast_ip = video_split_info->multicast_ip();
		m_multicast_port = video_split_info->multicast_port();
		m_split_method = video_split_info->split_method();
		// 判断一下路数
		if (video_split_info->camera_group_size() > 10 || video_split_info->camera_group_size() < 2)
		{
			WARNING_EX_LOG("camera group [size = %u]", video_split_info->camera_group_size());
			return EWebVideoSplitCameraGroupNum;
		}
		m_split_video_channels = video_split_info->camera_group_size();
		m_split_video_lock_1080p = video_split_info->lock_1080p();
		m_overlay = video_split_info->overlay();

		m_out_video_width = video_split_info->out_video_width();
		m_out_video_height = video_split_info->out_video_height();
		{
			m_osd.x = video_split_info->osd_info().x() * m_out_video_width;
			m_osd.y = video_split_info->osd_info().y() * m_out_video_height;
			m_osd.fontsize = video_split_info->osd_info().font_size();
			//m_osd.x = video_split_info->osd_info().x();
			//std::string osd_text = video_split_info->osd_info().font_text();
			m_osd.text = video_split_info->osd_info().font_text();
			NORMAL_EX_LOG("[osd  font_size = %u][x = %s]", m_osd.fontsize, std::to_string(m_osd.x).c_str());
		}


		// camera info arrays 
		for (size_t i = 0; i < video_split_info->camera_group_size(); ++i)
		{
			ccamera_info camera_info;
			camera_info.x = video_split_info->camera_group(i).x();
			camera_info.y = video_split_info->camera_group(i).y();
			camera_info.w = video_split_info->camera_group(i).w();
			camera_info.h = video_split_info->camera_group(i).h();
			camera_info.index = video_split_info->camera_group(i).index();
			// 查询URL
			const CameraInfo * camera_info_ptr = g_camera_info_mgr.get_camera_info(video_split_info->camera_group(i).camera_id());
			if (!camera_info_ptr)
			{
				WARNING_EX_LOG("[video_splist  id = %u]not find [camera id = %u]", video_split_info->id(), video_split_info->camera_group(i).camera_id());
				return EWebNotFindCameraId;
			}
			else
			{
				camera_info.url = "udp://@" + camera_info_ptr->address() + ":" + std::to_string(camera_info_ptr->port());
			}
			m_camera_infos.push_back(camera_info);
			//camera_info.url = video_split_info->camera_group(i).();
		}
		

		//m_encoder_frame_ptr = av_frame_alloc();
		m_stoped = false;
		m_thread = std::thread(&cvideo_splist::_pthread_work, this);

		return EWebSuccess;
	}
	void cvideo_splist::update(uint32 uDateTime)
	{
	}
	void cvideo_splist::destroy()
	{
		m_stoped = true;
		for (size_t i = 0; i < m_decode_pthread.size(); ++i)
		{
			if (m_decode_pthread[i].joinable())
			{
				m_decode_pthread[i].join();
			}
		}
		m_decode_pthread.clear();
		NORMAL_EX_LOG("");
		if (m_thread.joinable())
		{
			m_thread.join();
		}
		NORMAL_EX_LOG("");
		// filter 

		if (m_filter_graph_ptr)
		{
			
		//	m_filter_graph_ptr = NULL;
			for (size_t i = 0; i < m_buffers_ctx_ptr.size(); ++i)
			{
				if (m_buffers_ctx_ptr[i])
				{
					::avfilter_free(m_buffers_ctx_ptr[i]);
				}
				if (m_buffers_crop_ctx_ptr[i])
				{
					::avfilter_free(m_buffers_crop_ctx_ptr[i]);
				}
				if (m_buffers_scale_ctx_ptr[i])
				{
					::avfilter_free(m_buffers_scale_ctx_ptr[i]);
				}
			}
			m_buffers_ctx_ptr.clear();
			m_buffers_crop_ctx_ptr.clear();
			m_buffers_scale_ctx_ptr.clear();
			
			::avfilter_free(m_hstack_ctx_ptr);
			::avfilter_free(m_buffersink_ctx_ptr);
			m_hstack_ctx_ptr = NULL;
			m_buffersink_ctx_ptr = NULL;
			if (m_osd_ctx_ptr)
			{
				::avfilter_free(m_osd_ctx_ptr);
			}
			::avfilter_graph_free(&m_filter_graph_ptr);
			//m_osd_ctx_ptr = NULL;
			//m_osd_ctx_ptr = NULL;
			
		}
		NORMAL_EX_LOG("");
		for (size_t i = 0; i < m_decodes.size(); ++i)
		{
			if (m_decodes[i])
			{
				m_decodes[i]->destroy();
				delete m_decodes[i];
				NORMAL_EX_LOG("");
			}
		}
		NORMAL_EX_LOG("");
		/*if (m_decode_ptr1)
		{
			m_decode_ptr1->destroy();
			delete m_decode_ptr1;

		}
		if (m_decode_ptr2)
		{
			m_decode_ptr2->destroy();
			delete m_decode_ptr2;

		}*/
		m_camera_infos.clear();
		m_decodes.clear();
		if (m_encoder_ptr)
		{
			m_encoder_ptr->stop();
			m_encoder_ptr->destroy();
			delete m_encoder_ptr;
			m_encoder_ptr = NULL;
		}
		NORMAL_EX_LOG("");
		if (m_filter_frame_ptr)
		{
			::av_frame_free(&m_filter_frame_ptr);
			m_filter_frame_ptr = NULL;
		}
		
	}
	void cvideo_splist::callback_decoder_failed()
	{
		g_video_split_mgr.push_stop_video(m_video_split_channel);
	}
	bool cvideo_splist::_init_decodes(uint32 gpu_index)
	{
		for (int32 i = 0; i < m_camera_infos.size(); ++i)
		{
			cdecode* decoder_ptr = new cdecode(); 
			if (!decoder_ptr->init(gpu_index, m_camera_infos[i].url.c_str(), i, this))
			{
				decoder_ptr->destroy();
				delete decoder_ptr;
				WARNING_EX_LOG("[m_video_split_name = %s][%u] not open  [camera = %s]", m_video_split_name.c_str(),i, m_camera_infos[i].url.c_str());
				return false;
			}
			
			 m_decodes.push_back (decoder_ptr);
		}
		return true;
	}
	bool cvideo_splist::_init_filter()
	{
		//return true;
		int32 ret = 0;
		if (m_stoped)
		{
			WARNING_EX_LOG("[m_video_split_name = %s]", m_video_split_name.c_str());
			return false;
		}
		if (m_filter_graph_ptr)
		{
			WARNING_EX_LOG("[m_video_split_name = %s] filter graph != NULL", m_video_split_name.c_str());
			return false;
		}
		// alloc 
		m_filter_graph_ptr = ::avfilter_graph_alloc();
		if (!m_filter_graph_ptr)
		{
			WARNING_EX_LOG("[m_video_split_name = %s] alloc global filter failed !!!", m_video_split_name.c_str());

			return false;
		}

		/////   hstatck 
		std::string hstack_str = "inputs=" + std::to_string(m_camera_infos.size());
		std::string video_split_method = "hstack";
		if (m_overlay)
		{
			video_split_method = "overlay";
		}
		else
		{
			if (m_split_method)
			{
				video_split_method = "vstack";
			}
			//多路视频拼接 xstack是
		}
		ret = avfilter_graph_create_filter(&m_hstack_ctx_ptr, ::avfilter_get_by_name(video_split_method.c_str()), video_split_method.c_str(), hstack_str.c_str(), NULL, m_filter_graph_ptr);
		if (0 > ret)
		{
			//char buf[1024] = { 0 };
			//av_strerror(ret, buf, sizeof(buf) - 1);
			WARNING_EX_LOG("avfilter_graph_create_filter hstack failed !!! (%s)", ffmpeg_util::make_error_string(ret));
			//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_graph_create_filter  failed! :" << buf << std::endl;
			return false;
		}
		//osd
		//AVDictionary* options = NULL;
		//{
		//
		//	// 设置drawtext的基本参数，例如文本内容、字体、颜色等
		//	
		//	av_dict_set(&options, "text", "Hello, World!", 0);
		//	av_dict_set(&options, "fontfile", "simkai.ttf", 0);
		//	av_dict_set(&options, "fontcolor", "red", 0);
		//	av_dict_set(&options, "fontsize", "24", 0);
		//}
		//fontfile=simkai.ttf:fontcolor=red:fontsize=100:x=0:y=0:text='chensong'
		if (m_osd.text.size() > 0)
		{
			//std::string text = 
			std::string osd_args = "fontfile="+g_cfg.get_string(ECI_DataPath)+"/WenQuanYiMicroHei.ttf:fontcolor=white:fontsize=" + std::to_string(m_osd.fontsize)
				+ ":x=" + std::to_string(m_osd.x ) + ":y=" + std::to_string(m_osd.y  ) + ":text='" + osd_text_str(m_osd.text) + "'";
			ret = avfilter_graph_create_filter(&m_osd_ctx_ptr, ::avfilter_get_by_name("drawtext"),
				"drawtext", osd_args.c_str(), NULL, m_filter_graph_ptr);
			NORMAL_EX_LOG("[osd_args = %s]", osd_args.c_str());
			if (0 > ret)
			{
				// 清理字典
				//av_dict_free(&options);
				WARNING_EX_LOG("avfilter_graph_create_filter osd failed !!! (%s)", ffmpeg_util::make_error_string(ret));

				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_graph_create_filter  failed! :" << buf << std::endl;
				return false;
			}
		}
		else
		{
			WARNING_EX_LOG("not find osd !!!");
		}
		// 清理字典
		//av_dict_free(&options);
		// sink
		ret = avfilter_graph_create_filter(&m_buffersink_ctx_ptr, ::avfilter_get_by_name("buffersink"), "out", NULL, NULL, m_filter_graph_ptr);
		if (0 > ret)
		{
			 
			WARNING_EX_LOG("avfilter_graph_create_filter buffersink failed !!! (%s)", ffmpeg_util::make_error_string(ret));

			//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_graph_create_filter  failed! :" << buf << std::endl;
			return false;
		}


		m_buffers_ctx_ptr.resize(m_camera_infos.size());
		m_buffers_crop_ctx_ptr.resize(m_camera_infos.size());
		m_buffers_scale_ctx_ptr.resize(m_camera_infos.size());
		
		//TODO@chensong 2023-02-21 拼接效果视频放置每个宽度或者高度
		uint32 split_one_width = m_out_video_width;
		uint32 split_one_height = m_out_video_height;
		if (!m_overlay) 
		{
			if (m_split_method)
			{
				split_one_height = m_out_video_height / m_camera_infos.size(); 
			}
			else
			{
				split_one_width = m_out_video_width / m_camera_infos.size();
			}
		}

		for (size_t i = 0; i < m_buffers_ctx_ptr.size(); ++i)
		{
			std::string args;
			std::string in;

			uint32 crop_x =   m_camera_infos[i].x* m_decodes[i]->m_codec_ctx_ptr->width;
			uint32 crop_y =   m_camera_infos[i].y* m_decodes[i]->m_codec_ctx_ptr->height;
			uint32 crop_width =   m_camera_infos[i].w* m_decodes[i]->m_codec_ctx_ptr->width;
			uint32 crop_height =   m_camera_infos[i].h* m_decodes[i]->m_codec_ctx_ptr->height;
			

			std::string crop_str = std::to_string(crop_width)+ ":" + std::to_string(crop_height)
				+ ":" + std::to_string(crop_x) + ":" + std::to_string(crop_y);// height:width:x:y/*"1910:1000:10:10";*/
			std::string scale_str = std::to_string(split_one_width) + ":" + std::to_string(split_one_height); //"1920:1080";
			in = "in" + std::to_string(i);
			NORMAL_EX_LOG("[video split name = %s][%u][crop = %s][scale = %s]", m_video_split_name.c_str(), i, crop_str.c_str(), scale_str.c_str());
			args.resize(1024);
			snprintf((char *)args.data(), 1024,	"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
			, m_decodes[i]->m_codec_ctx_ptr->width, m_decodes[i]->m_codec_ctx_ptr->height,
			m_decodes[i]->m_codec_ctx_ptr->pix_fmt,
				m_decodes[i]->m_ic_ptr->streams[m_decodes[i]->m_video_stream_index]->time_base.num, 
				m_decodes[i]->m_ic_ptr->streams[m_decodes[i]->m_video_stream_index]->time_base.den,
				m_decodes[i]->m_codec_ctx_ptr->sample_aspect_ratio.num,
				m_decodes[i]->m_codec_ctx_ptr->sample_aspect_ratio.den);
			// src buffer 0 

			ret = ::avfilter_graph_create_filter(&m_buffers_ctx_ptr[i], ::avfilter_get_by_name("buffer"), in.c_str(), args.c_str(), NULL/*buffer 用户数据*/, m_filter_graph_ptr);
			if (ret < 0)
			{
				WARNING_EX_LOG("in -->avfilter graph create filter failed !!!");
				return ret;
			}
			// scale_cuda
			ret = ::avfilter_graph_create_filter(&m_buffers_crop_ctx_ptr[i], ::avfilter_get_by_name("crop"), "crop", crop_str.c_str(), NULL/*buffer 用户数据*/, m_filter_graph_ptr);
			if (ret < 0)
			{
				WARNING_EX_LOG("in -->avfilter graph create filter failed !!!");
				return ret;
			}

			
			ret = ::avfilter_graph_create_filter(&m_buffers_scale_ctx_ptr[i], ::avfilter_get_by_name("scale"), "scale", scale_str.c_str(), NULL/*buffer 用户数据*/, m_filter_graph_ptr);
			if (ret < 0)
			{
				WARNING_EX_LOG("in -->avfilter graph create filter failed !!!");
				return ret;
			}

			// 配置  处理流程
			ret = avfilter_link( m_buffers_ctx_ptr[i], 0, m_buffers_crop_ctx_ptr[i], 0);
			if (0 > ret)
			{
				WARNING_EX_LOG("[i = %d] avfilter_link  failed!!! : (%s)", i, ffmpeg_util::make_error_string(ret));
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				return false;
			}
			ret = avfilter_link(m_buffers_crop_ctx_ptr[i], 0, m_buffers_scale_ctx_ptr[i], 0);
			if (0 > ret)
			{
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				WARNING_EX_LOG("[i = %d] avfilter_link  failed!!! : (%s)", i, ffmpeg_util::make_error_string(ret));
				return false;
			}
			ret = avfilter_link(m_buffers_scale_ctx_ptr[i], 0, m_hstack_ctx_ptr, i);
			if (0 > ret)
			{
				//WARNING_EX_LOG("");
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				WARNING_EX_LOG("[i = %d] avfilter_link  failed!!! : (%s)", i, ffmpeg_util::make_error_string(ret));

				return false;
			}
		}
		if (m_osd_ctx_ptr)
		{
			ret = avfilter_link(m_hstack_ctx_ptr, 0, m_osd_ctx_ptr, 0);
			if (0 > ret)
			{
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				WARNING_EX_LOG(" osd avfilter_link  failed!!! : (%s)", ffmpeg_util::make_error_string(ret));

				return false;
			}

			ret = avfilter_link(m_osd_ctx_ptr, 0, m_buffersink_ctx_ptr, 0);
			if (0 > ret)
			{
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				WARNING_EX_LOG("[ideo_split_vname = %s]  avfilter_link  failed!!! : (%s)", m_video_split_name.c_str(), ffmpeg_util::make_error_string(ret));

				return false;
			}
		}
		else
		{
			ret = avfilter_link(m_hstack_ctx_ptr, 0, m_buffersink_ctx_ptr, 0);
			if (0 > ret)
			{
				//char buf[1024] = { 0 };
				//av_strerror(ret, buf, sizeof(buf) - 1);
				//std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
				WARNING_EX_LOG("[ideo_split_vname = %s]  avfilter_link  failed!!! : (%s)", m_video_split_name.c_str(), ffmpeg_util::make_error_string(ret));

				return false;
			}
		}
		
		//graph 生效
		ret = ::avfilter_graph_config(m_filter_graph_ptr, NULL);
		if (ret < 0)
		{
			WARNING_EX_LOG("[ideo_split_vname = %s] filter graph config failed (%s)!!!\n", m_video_split_name.c_str() , chen::ffmpeg_util::make_error_string(ret));
			return false;
		}
		return true;
	}



	void cvideo_splist::_pthread_work()
	{
		if (!_open())
		{
			WARNING_EX_LOG("open [channel_name = %s] failed !!!", m_video_split_channel.c_str());
			g_video_split_mgr.set_channel_name_status(m_id, 0);
			
			return;
		}
		g_video_split_mgr.set_channel_name_status(m_id, 1);
		//SetThreadDescription(GetCurrentThread(), (PCWSTR)m_video_split_name.c_str());
		//pthread_setname_np(pthread_self(), m_video_split_name.c_str());
		//NORMAL_EX_LOG("");
		int32_t ret = 0;
		AVFrame* frame_ptr = NULL;
		uint64 dts = 0;
		uint64 pts = 0;
		uint32  d_ms =   1000   / 45;
		for (int32 i = 0; i < m_decodes.size(); ++i)
		{
			m_decode_pthread.emplace_back(std::thread(&cvideo_splist::_pthread_decodec, this, i));
		} 
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds ms_frame_count = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		uint32 frame_fff = 0;

		ctime_stamp  tp;
		auto timestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch())
			.count();
		size_t cnt = 0;
		  m_frame_count_num = 0;
		 // uint64 frame_dps = 0;
		  m_frame_total_count = 0;
		//m_filter_frame_ptr = NULL;
		while (!m_stoped && m_buffersink_ctx_ptr)
		{
 //goto_init_frame:
			 if (!m_filter_frame_ptr)
			{
				m_filter_frame_ptr = ::av_frame_alloc();
			}
			if (!m_filter_frame_ptr)
			{
				WARNING_EX_LOG("[video_channel = %s]alloc frame failed !!!", m_video_split_channel.c_str());
				continue;
			} 

			//for (size_t i = 0; i < m_decodes.size(); ++i)
			//{
			//	if (m_decodes[i]->retrieve(frame_ptr))
			//	{
			//		frame_ptr->pts = global_calculate_pts(m_decodes[i]->get_number_frame(), 25);//m_decodes[0]->get_index_pts(m_decodes[decodec_id]->get_number_frame());
			//		frame_ptr->pkt_dts = frame_ptr->pts;
			//		ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[i], frame_ptr);
			//		//ret = ::av_buffersrc_write_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
			//		//ret = ::av_buffersrc_add_frame_flags(m_buffers_ctx_ptr[decodec_id], frame_ptr, AV_BUFFERSRC_FLAG_PUSH);
			//		if (ret < 0)
			//		{
			//			WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", i, chen::ffmpeg_util::make_error_string(ret));

			//		}
			//	//	cnt++;

			//	}
			//	::av_frame_unref(frame_ptr);
			//}
			 
			//NORMAL_EX_LOG("");
			if (m_stoped)
			{
				//中退出时啦 ^_^
				break;
			}
			 
 
			// get buffersink filer frame --> 
			{
				
				// 上面问题崩溃 我解决方案是修改ffmpeg源码
				//ret = -1;
				//while (ret <0 && !m_stoped)
				{
					//clock_guard lock(g_avfilter_lock);

				//	ret = ::av_buffersink_get_frame_flags(m_buffersink_ctx_ptr, m_filter_frame_ptr, AV_BUFFERSINK_FLAG_PEEK);
					//ret = ::av_buffersink_get_frame_flags(m_buffersink_ctx_ptr, m_filter_frame_ptr, AV_BUFFERSINK_FLAG_NO_REQUEST);
					ret = ::av_buffersink_get_frame(m_buffersink_ctx_ptr, m_filter_frame_ptr);

				}
				 
			}
			if (ret < 0 && m_filter_frame_ptr)
			{
				::av_frame_unref(m_filter_frame_ptr);
				// filter 错误啦 ^_^
				// continue;
			}
			//NORMAL_EX_LOG("---> frame -- encoder ");
			// 放到编码器中去编码啦 ^_^
			if (!m_stoped && ret >= 0 && m_filter_frame_ptr)
			{
				 
				if (++m_frame_count_num > g_frame_cryle)
				{
					m_frame_count_num = 0;
					++m_frame_total_count;
					NORMAL_EX_LOG("main  frame cylec = %u", m_frame_total_count);
				}
				pts =  global_calculate_pts(m_frame_count_num, 12);  //m_decodes[0]->get_index_pts(frame_count_num);
				dts = pts;//m_decodes[0]->get_index_dts(frame_count_num);
				
			 	m_encoder_ptr->push_frame(m_filter_frame_ptr, dts, pts);
			}
			if (m_filter_frame_ptr)
			{
				::av_frame_unref(m_filter_frame_ptr);
				//::av_frame_free(&m_filter_frame_ptr);
				//m_filter_frame_ptr = NULL;
			}
			

			{
				 
				std::chrono::milliseconds encoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
				std::chrono::milliseconds  diff_ms = encoder_ms - ms;
				ms = encoder_ms;
				if (ret >= 0)
				{
					cnt++;
					
				}
				auto timestamp_curr = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch())
					.count();
				if (timestamp_curr - timestamp > 1000) {
					//RTC_LOG(LS_INFO) << "FPS: " << cnt;
					NORMAL_EX_LOG("main  send  fps = %u", cnt);
					NORMAL_EX_LOG("frame [filter number = %u] [   %u][%u][d_ms = %u] pts = [%u]", m_frame_count_num, m_decodes[0]->get_number_frame(), m_decodes[0]->get_number_frame(), d_ms, diff_ms.count());

					cnt = 0;
					timestamp = timestamp_curr;
				}
				/*if (diff_ms.count() != 0)
				{
					NORMAL_EX_LOG("main ms = %u", diff_ms.count());
				}*/
				if (diff_ms.count() < d_ms)
				{
					  std::this_thread::sleep_for(std::chrono::milliseconds(d_ms - diff_ms.count()));
					  ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					 	 std::chrono::system_clock::now().time_since_epoch());
				}
				else
				{
					 
					  ms += std::chrono::milliseconds(diff_ms.count() - d_ms);
				}
				ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
			}

		}
		//NORMAL_EX_LOG("");
		//for (int32 i = 0; i < 11; ++i)
		//{
		//	if (frame_ptr[i])
		//	{
		//		::av_frame_unref(frame_ptr[i]);
		//		//::av_frame_free(&frame_ptr);
		//		frame_ptr[i] = NULL;
		//	}
		//}
		
		 
	}
	bool cvideo_splist::_open()
	{
		if (!_init_decodes(m_gpu_index))
		{
			return false;
		}
		if (!_init_filter())
		{
			return false;
		}
		m_encoder_ptr = new cencoder();
		if (!m_encoder_ptr)
		{
			WARNING_EX_LOG("split video name = [%s], alloc encoder failed !!!", m_video_split_name.c_str());
			return false;
		}

		std::string encoder_url = std::string("udp://@" + m_multicast_ip + ":" + std::to_string(m_multicast_port));
		if (!m_encoder_ptr->init(m_gpu_index, m_multicast_ip.c_str(), m_multicast_port, m_out_video_width, m_out_video_height))
		{
			WARNING_EX_LOG("video split name = [%s] encoder init (%s)failed !!!", m_video_split_name.c_str(), encoder_url.c_str());
			return false;
		}
		// 编码器
		return true;
	}
	void cvideo_splist::_pthread_decodec(uint32 decodec_id)
	{
		int32_t ret = 0;
		AVFrame* frame_ptr = NULL;
		uint64 dts = 0;
		  auto timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch())
					.count();
				  size_t cnt = 0;
		uint64 pts = 0;
		uint32  d_ms = 1000 /  (m_decodes[decodec_id]->get_fps() + g_cfg.get_uint32(ECI_VideoDecoderFrame));
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		int64 frmame_fps = 0;
		int64 frame_total_count_fps = 0;
		while (!m_stoped && m_buffersink_ctx_ptr)
		{
			if (m_decodes[decodec_id]->retrieve(frame_ptr))
			{

				//	if (m_decodes[decodec_id]->get_number_frame() > )
					//if (m_decodes[decodec_id]->get_number_frame() % 2 == 0)
					// TODO@chensong 20240929 消费能力太差了
				if ((((frmame_fps + (g_frame_cryle * frame_total_count_fps)) - (m_frame_count_num + (g_frame_cryle * m_frame_total_count))) < 5) /*&& (m_decodes[decodec_id]->get_number_frame() % g_cfg.get_uint32(ECI_VideoSkipFrameNum) == 0 )*/)
				{
					++frmame_fps;
					if (frmame_fps > g_frame_cryle)
					{
						++frame_total_count_fps;
						frmame_fps = 0;
						NORMAL_EX_LOG("decoder [ %u] frame cylec = %u", decodec_id, frame_total_count_fps);


					}
					frame_ptr->pts = global_calculate_pts(frmame_fps, 12);//m_decodes[0]->get_index_pts(m_decodes[decodec_id]->get_number_frame());
					frame_ptr->pkt_dts = frame_ptr->pts;
					ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
					//ret = ::av_buffersrc_write_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
					//ret = ::av_buffersrc_add_frame_flags(m_buffers_ctx_ptr[decodec_id], frame_ptr, AV_BUFFERSRC_FLAG_PUSH);
					if (ret < 0)
					{
						WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", decodec_id, chen::ffmpeg_util::make_error_string(ret));

					}
				}
				else if (((frmame_fps + (g_frame_cryle * frame_total_count_fps)) - (m_frame_count_num + (g_frame_cryle * m_frame_total_count))) && (m_decodes[decodec_id]->get_number_frame() % g_cfg.get_uint32(ECI_VideoSkipFrameNum) == 0))
				{
					++frmame_fps;
					if (frmame_fps > g_frame_cryle)
					{
						frmame_fps = 0;
						++frame_total_count_fps;
						NORMAL_EX_LOG("decoder [ %u] frame cylec = %u", decodec_id, frame_total_count_fps);
					}
					frame_ptr->pts = global_calculate_pts(frmame_fps, 12);//m_decodes[0]->get_index_pts(m_decodes[decodec_id]->get_number_frame());
					frame_ptr->pkt_dts = frame_ptr->pts;
					ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
					//ret = ::av_buffersrc_write_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
					//ret = ::av_buffersrc_add_frame_flags(m_buffers_ctx_ptr[decodec_id], frame_ptr, AV_BUFFERSRC_FLAG_PUSH);
					if (ret < 0)
					{
						WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", decodec_id, chen::ffmpeg_util::make_error_string(ret));

					}
				}

				cnt++;

			}
			else if (m_decodes[decodec_id]->get_reconnect())
			{
				::av_frame_unref(frame_ptr);
				m_decodes[decodec_id]->destroy();
				if (!m_decodes[decodec_id]->init(m_gpu_index, m_camera_infos[decodec_id].url.c_str(), decodec_id, this))
				{
					m_decodes[decodec_id]->destroy();
					//	delete decoder_ptr;
					WARNING_EX_LOG("[m_video_split_name = %s][%u] reconnect  not open  [camera = %s]", m_video_split_name.c_str(), decodec_id, m_camera_infos[decodec_id].url.c_str());
					//return false;


				}
				else
				{
					NORMAL_EX_LOG("[m_video_split_name = %s][%u] reconnect  open  [camera = %s]", m_video_split_name.c_str(), decodec_id, m_camera_infos[decodec_id].url.c_str());
					continue;
				}


			}
			::av_frame_unref(frame_ptr);

			if (m_stoped)
			{
				break;
			}
			//NORMAL_EX_LOG("");
			if (m_stoped)
			{
				//中退出时啦 ^_^
				break;
			}
			{
				auto timestamp_curr = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch())
					.count();
				if (timestamp_curr - timestamp > 1000) {
					//RTC_LOG(LS_INFO) << "FPS: " << cnt;
					NORMAL_EX_LOG("chiled[%u] fps = %u", decodec_id, cnt);
					cnt = 0;
					timestamp = timestamp_curr;
				}
				std::chrono::milliseconds encoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
				std::chrono::milliseconds  diff_ms = encoder_ms - ms;
				
				if (diff_ms.count() < d_ms)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(d_ms - diff_ms.count()));
				}
				//else 
				{
				 	ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					 std::chrono::system_clock::now().time_since_epoch());
					// ms += std::chrono::milliseconds(diff_ms.count() - d_ms);
				}
				

				
			}
		}
		::av_frame_unref(frame_ptr);
	}
}