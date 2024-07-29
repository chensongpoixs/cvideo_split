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
#include "cvideo_split_mgr.h"
namespace chen {
	static				std::mutex   g_avfilter_lock;

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
	bool cvideo_splist::_init_decodes(uint32 gpu_index)
	{
		for (int32 i = 0; i < m_camera_infos.size(); ++i)
		{
			cdecode* decoder_ptr = new cdecode(); 
			if (!decoder_ptr->init(gpu_index, m_camera_infos[i].url.c_str(), i))
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
		uint32  d_ms =   1000   / 40;
		for (int32 i = 0; i < m_decodes.size(); ++i)
		{
			m_decode_pthread.emplace_back(std::thread(&cvideo_splist::_pthread_decodec, this, i));
		}
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds ms_frame_count = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		uint32 frame_fff = 0;
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
			if (false)
			{
				int64  pts_s = 0;
				for (int32 i = 0 ; i < m_decodes.size(); ++i)
				{
						if (m_decodes[i]->retrieve(frame_ptr))
						{
							if (i == 0)
							{
								pts = m_decodes[0]->get_pts();
								dts = m_decodes[0]->get_dts();
								pts_s = frame_ptr->pts;
							} 
							frame_ptr->pts = pts_s;
							ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[i], frame_ptr); 
							if (ret < 0)
							{
								WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", i, chen::ffmpeg_util::make_error_string(ret));
									 
							}
						}  
						::av_frame_unref(frame_ptr);

						if (m_stoped)
						{
							break;
						} 
				}
			}
			//NORMAL_EX_LOG("");
			if (m_stoped)
			{
				//中退出时啦 ^_^
				break;
			}
			 
 
			// get buffersink filer frame --> 
			{
				// TODO@chensong 20240403 OSD字体库thread 是不安全的啦 ！！！
				 
				// TODO@chensong 20240725  linux  core  
				/*
				Program terminated with signal SIGSEGV, Segmentation fault.
				#0  scale_eval_dimensions (ctx=0x7fa1d91e5080) at libavfilter/vf_scale.c:465
				465     libavfilter/vf_scale.c: 没有那个文件或目录.
				[Current thread is 1 (Thread 0x7fa1e2cf9700 (LWP 106627))]
				(gdb) bt
				#0  scale_eval_dimensions (ctx=0x7fa1d91e5080) at libavfilter/vf_scale.c:465
				#1  config_props (outlink=outlink@entry=0x7fa1d91f2780) at libavfilter/vf_scale.c:526
				#2  0x0000563442c4d465 in scale_frame (link=link@entry=0x7fa1d91f25c0, in=<optimized out>,
					frame_out=frame_out@entry=0x7fa1e2cf8d20) at libavfilter/vf_scale.c:802
				#3  0x0000563442c4df2c in filter_frame (link=link@entry=0x7fa1d91f25c0, in=<optimized out>)
					at libavfilter/vf_scale.c:911
				#4  0x0000563442b0562a in ff_filter_frame_framed (frame=<optimized out>, link=0x7fa1d91f25c0)
					at libavfilter/avfilter.c:969
				#5  ff_filter_frame_to_filter (link=0x7fa1d91f25c0) at libavfilter/avfilter.c:1123
				#6  ff_filter_activate_default (filter=<optimized out>) at libavfilter/avfilter.c:1182
				#7  ff_filter_activate (filter=<optimized out>) at libavfilter/avfilter.c:1341
				#8  0x0000563442b08437 in ff_filter_graph_run_once (graph=<optimized out>) at libavfilter/avfiltergraph.c:1353
				#9  0x0000563442b08dbd in get_frame_internal (ctx=0x7fa1d91e3b80, frame=0x7fa1da84ff80, flags=flags@entry=0,
					samples=<optimized out>) at libavfilter/buffersink.c:139
				#10 0x0000563442b08f86 in av_buffersink_get_frame_flags (ctx=<optimized out>, frame=<optimized out>,
					flags=flags@entry=0) at libavfilter/buffersink.c:150
				#11 0x0000563442b08f9b in av_buffersink_get_frame (ctx=<optimized out>, frame=<optimized out>)
					at libavfilter/buffersink.c:98
				#12 0x000056344211a6e5 in chen::cvideo_splist::_pthread_work (this=0x563446edf1c0)
					at /home/chensong/Work/cvideo_split/cvideo_split/cvideo_split.cpp:547
				#13 0x00007fa1f2e50df4 in ?? () from /lib/x86_64-linux-gnu/libstdc++.so.6
				#14 0x00007fa1f2f6a609 in start_thread (arg=<optimized out>) at pthread_create.c:477
				#15 0x00007fa1f2b3b353 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
				*/
				/*
				源文件   可能输入的inlink值获取不到desc是空指针
				 437 static int scale_eval_dimensions(AVFilterContext *ctx)
				 438 {
				 439     ScaleContext *scale = ctx->priv;
				 440     const char scale2ref = ctx->filter == &ff_vf_scale2ref;
				 441     const AVFilterLink *inlink = scale2ref ? ctx->inputs[1] : ctx->inputs[0];
				 442     const AVFilterLink *outlink = ctx->outputs[0];
				 443     const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(inlink->format);
				 444     const AVPixFmtDescriptor *out_desc = av_pix_fmt_desc_get(outlink->format);
				 445     char *expr;
				 446     int eval_w, eval_h;
				 447     int ret;
				 448     double res;
				 449     const AVPixFmtDescriptor *main_desc;
				 450     const AVFilterLink *main_link;
				 451
				 452     if (scale2ref) {
				 453         main_link = ctx->inputs[0];
				 454         main_desc = av_pix_fmt_desc_get(main_link->format);
				 455     }
				 456
				 457     scale->var_values[VAR_IN_W]  = scale->var_values[VAR_IW] = inlink->w;
				 458     scale->var_values[VAR_IN_H]  = scale->var_values[VAR_IH] = inlink->h;
				 459     scale->var_values[VAR_OUT_W] = scale->var_values[VAR_OW] = NAN;
				 460     scale->var_values[VAR_OUT_H] = scale->var_values[VAR_OH] = NAN;
				 461     scale->var_values[VAR_A]     = (double) inlink->w / inlink->h;
				 462     scale->var_values[VAR_SAR]   = inlink->sample_aspect_ratio.num ?
				 463         (double) inlink->sample_aspect_ratio.num / inlink->sample_aspect_ratio.den : 1;
				 464     scale->var_values[VAR_DAR]   = scale->var_values[VAR_A] * scale->var_values[VAR_SAR];
				 465     scale->var_values[VAR_HSUB]  = 1 << desc->log2_chroma_w;
				 466     scale->var_values[VAR_VSUB]  = 1 << desc->log2_chroma_h;
				 467     scale->var_values[VAR_OHSUB] = 1 << out_desc->log2_chroma_w;
				 468     scale->var_values[VAR_OVSUB] = 1 << out_desc->log2_chroma_h;
				 469
				 470     if (scale2ref) {
				 471         scale->var_values[VAR_S2R_MAIN_W] = main_link->w;
				 472         scale->var_values[VAR_S2R_MAIN_H] = main_link->h;
				 473         scale->var_values[VAR_S2R_MAIN_A] = (double) main_link->w / main_link->h;
				 474         scale->var_values[VAR_S2R_MAIN_SAR] = main_link->sample_aspect_ratio.num ?
				 475             (double) main_link->sample_aspect_ratio.num / main_link->sample_aspect_ratio.den : 1;
				 476         scale->var_values[VAR_S2R_MAIN_DAR] = scale->var_values[VAR_S2R_MDAR] =
				 477             scale->var_values[VAR_S2R_MAIN_A] * scale->var_values[VAR_S2R_MAIN_SAR];
				 478         scale->var_values[VAR_S2R_MAIN_HSUB] = 1 << main_desc->log2_chroma_w;
				 479         scale->var_values[VAR_S2R_MAIN_VSUB] = 1 << main_desc->log2_chroma_h;
				 480     }
				 481
				 482     res = av_expr_eval(scale->w_pexpr, scale->var_values, NULL);
				 483     eval_w = scale->var_values[VAR_OUT_W] = scale->var_values[VAR_OW] = (int) res == 0 ? inlink->w : (int) res;
				 484
				 485     res = av_expr_eval(scale->h_pexpr, scale->var_values, NULL);
				 486     if (isnan(res)) {
				 487         expr = scale->h_expr;
				 488         ret = AVERROR(EINVAL);
				 489         goto fail;
				 490     }
				 491     eval_h = scale->var_values[VAR_OUT_H] = scale->var_values[VAR_OH] = (int) res == 0 ? inlink->h : (int) res;
				 492
				 493     res = av_expr_eval(scale->w_pexpr, scale->var_values, NULL);
				 494     if (isnan(res)) {
				 495         expr = scale->w_expr;
				 496         ret = AVERROR(EINVAL);
				*/
				//判断scae 函数中inputs变量是否存在
				//for (  size_t inlink_i = 0; inlink_i < m_buffers_scale_ctx_ptr.size(); ++inlink_i)
				//{

				//	const AVFilterLink* inlink = m_buffers_scale_ctx_ptr[inlink_i]->inputs[0];
				//	if (inlink == NULL)
				//	{
				//		WARNING_EX_LOG("push = [%s] inlink == NULL , scale get inlink format(%u) failed !!!", std::string(m_multicast_ip + std::to_string(m_multicast_port)).c_str(), inlink_i);
				//		std::this_thread::sleep_for(std::chrono::milliseconds(20));
				//		goto goto_init_frame;
				//	}
				//	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(inlink->format));
				//	if (desc == NULL)
				//	{
				//		WARNING_EX_LOG("push = [%s] scale get inlink format(%u) failed !!!",  std::string(m_multicast_ip + std::to_string(m_multicast_port)).c_str(), inlink_i);
				//		std::this_thread::sleep_for(std::chrono::milliseconds(20));
				//		goto goto_init_frame;
				//	}
				//	//if (av_pix_fmt_desc_get(inlink->format))
				//}
				// 
				// 上面问题崩溃 我解决方案是修改ffmpeg源码
				clock_guard lock(g_avfilter_lock);
				try
				{
					if ((ret = ::av_buffersink_get_frame(m_buffersink_ctx_ptr, m_filter_frame_ptr)) < 0)
					{
						//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						{
							//NORMAL_EX_LOG("");
							// 需要继续处理啦
							::av_frame_unref(m_filter_frame_ptr);
							//continue;
						}
						//filter error 
						//WARNING_EX_LOG("[video_channel = %s][buffersink get frame = %s]", m_video_split_channel, ffmpeg_util::make_error_string(ret));
						//continue;
					}
				}
				catch (const std::exception&  e)
				{
					WARNING_EX_LOG("video split [name = %s] exception =[%s][filter error = %s] failed !!!", m_video_split_name.c_str(),e.what(),  ffmpeg_util::make_error_string(ret));
					continue;
				}
				catch (...)
				{
					WARNING_EX_LOG("video split [name = %s] exception [filter error = %s] failed !!!", m_video_split_name.c_str(), ffmpeg_util::make_error_string(ret));
					continue;
				}
			}
			if (ret < 0)
			{
				// filter 错误啦 ^_^
				//filter error 
				//WARNING_EX_LOG("[video_channel = %s][buffersink get frame = %s]", m_video_split_channel, ffmpeg_util::make_error_string(ret));
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
				WARNING_EX_LOG("video split [name = %s] [filter error = %s] failed !!!", m_video_split_name.c_str(), ffmpeg_util::make_error_string(ret));
				continue;
			}
			//NORMAL_EX_LOG("---> frame -- encoder ");
			// 放到编码器中去编码啦 ^_^
			if (!m_stoped)
			{
				pts = m_decodes[0]->get_pts();
				dts = m_decodes[0]->get_dts();
			 	m_encoder_ptr->push_frame(m_filter_frame_ptr, dts, pts);
			}
			::av_frame_unref(m_filter_frame_ptr);

			{
				if (false)
				{
					++frame_fff;
					std::chrono::milliseconds diff_ms_frame_count = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch());
					std::chrono::milliseconds  diff_ms = diff_ms_frame_count - ms_frame_count;
					if (diff_ms.count() > 1000)
					{
						WARNING_EX_LOG("push frame = %u fps ", frame_fff);
						frame_fff = 0;
						ms_frame_count = diff_ms_frame_count;
					}
				}
				std::chrono::milliseconds encoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
				std::chrono::milliseconds  diff_ms = encoder_ms - ms;
				
				if (diff_ms.count() < d_ms)
				{
					 std::this_thread::sleep_for(std::chrono::milliseconds(d_ms - diff_ms.count()));
					 ms = std::chrono::duration_cast<std::chrono::milliseconds>(
						 std::chrono::system_clock::now().time_since_epoch());
				}
				//else
				{
					ms = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch());
					//ms += std::chrono::milliseconds(diff_ms.count() - d_ms);
				}

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
		uint64 pts = 0;
		uint32  d_ms = 1000 / (m_decodes[decodec_id]->get_fps() +15);
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		while (!m_stoped && m_buffersink_ctx_ptr)
		{
			{
				{
					if (m_decodes[decodec_id]->retrieve(frame_ptr))
					{
						//if (decodec_id == 0)
						//{
						//	//NORMAL_EX_LOG("pts = %u", m_decodes[0]->get_pts());
						//}
						frame_ptr->pts = m_decodes[0]->get_pts();
						ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[decodec_id], frame_ptr);
						if (ret < 0)
						{
							WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", decodec_id, chen::ffmpeg_util::make_error_string(ret));

						}
					}
					::av_frame_unref(frame_ptr);

					if (m_stoped)
					{
						break;
					}
				}
			}
			//NORMAL_EX_LOG("");
			if (m_stoped)
			{
				//中退出时啦 ^_^
				break;
			}
			{
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