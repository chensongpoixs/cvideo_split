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

namespace chen {
	bool cvideo_splist::init(const VideoSplitInfo* video_split_info)
	{
		if (!video_split_info)
		{
			return false;
		}
		m_stoped = false;
		m_video_split_name = video_split_info->split_channel_name();
		m_video_split_channel = video_split_info->split_channel_id();
		m_multicast_ip = video_split_info->multicast_ip();
		m_multicast_port = video_split_info->multicast_port();
		m_split_method = video_split_info->split_method();
		// 判断一下路数
		if (video_split_info->camera_group_size() > 10 || video_split_info->camera_group_size() < 2)
		{
			WARNING_EX_LOG("camera group [size = %u]", video_split_info->camera_group_size());
			return false;
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
			}
			else
			{
				camera_info.url = (camera_info_ptr->url());
			}
			m_camera_infos.push_back(camera_info);
			//camera_info.url = video_split_info->camera_group(i).();
		}
		if (!_init_decodes())
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
		if (!m_encoder_ptr->init(encoder_url.c_str(), m_out_video_width, m_out_video_height))
		{
			WARNING_EX_LOG("video split name = [%s] encoder init (%s)failed !!!", m_video_split_name.c_str(), encoder_url.c_str());
			return false;
		} 
		// 编码器

		//m_encoder_frame_ptr = av_frame_alloc();

		m_thread = std::thread(&cvideo_splist::_pthread_work, this);

		return true;
	}
	void cvideo_splist::update(uint32 uDateTime)
	{
	}
	void cvideo_splist::destroy()
	{
		m_stoped = true;
		if (m_thread.joinable())
		{
			m_thread.join();
		}

		// filter 

		if (m_filter_graph_ptr)
		{
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
			if (m_osd_ctx_ptr)
			{
				::avfilter_free(m_osd_ctx_ptr);
			}
			::avfilter_graph_free(&m_filter_graph_ptr);
			m_filter_graph_ptr = NULL;
		}

		for (size_t i = 0; i < m_decodes.size(); ++i)
		{
			if (m_decodes[i])
			{
				m_decodes[i]->destroy();
				delete m_decodes[i];
			}
		}
		m_camera_infos.clear();
		m_decodes.clear();
		if (m_encoder_ptr)
		{
			m_encoder_ptr->stop();
			m_encoder_ptr->destroy();
			delete m_encoder_ptr;
			m_encoder_ptr = NULL;
		}
	}
	bool cvideo_splist::_init_decodes()
	{
		for (int32 i = 0; i < m_camera_infos.size(); ++i)
		{
			cdecode* decoder_ptr = new cdecode();
			if (!decoder_ptr->init(m_camera_infos[i].url.c_str()))
			{
				decoder_ptr->destroy();
				delete decoder_ptr;
				WARNING_EX_LOG("[m_video_split_name = %s][%u] not open  [camera = %s]", m_video_split_name.c_str(),i, m_camera_infos[i].url.c_str());
				return false;
			}
			m_decodes.push_back(decoder_ptr);
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
		ret = avfilter_graph_create_filter(&m_hstack_ctx_ptr, ::avfilter_get_by_name("hstack"), "hstack", hstack_str.c_str(), NULL, m_filter_graph_ptr);
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
			std::string osd_args = "fontfile=simkai.ttf:fontcolor=red:fontsize=" + std::to_string(m_osd.fontsize)
				+ ":x=" + std::to_string(m_osd.x ) + ":y=" + std::to_string(m_osd.y  ) + ":text='" + m_osd.text + "'";
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

		AVFrame* filter_frame_ptr = ::av_frame_alloc();
		if (!filter_frame_ptr)
		{
			// 
			WARNING_EX_LOG("[video_channel = %s]alloc frame failed !!!", m_video_split_channel.c_str());
		}
		//NORMAL_EX_LOG("");
		int32_t ret = 0;
		AVFrame* frame_ptr = NULL;
		while (!m_stoped)
		{
			if (!filter_frame_ptr)
			{
				filter_frame_ptr = ::av_frame_alloc();
			}
			if (!filter_frame_ptr)
			{
				WARNING_EX_LOG("[video_channel = %s]alloc frame failed !!!", m_video_split_channel.c_str());
				continue;
			}
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			// add buffer filter -->
			// decoder 
			for (size_t i = 0; i < m_decodes.size(); ++i)
			{
				
				//if (m_decodes[i])
				//if (false)
				{
					std::chrono::milliseconds buffer_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
					if (m_decodes[i]->retrieve(frame_ptr) && !m_stoped)
					{
						std::chrono::milliseconds buffer_src_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch());

						std::chrono::milliseconds diff_ms = buffer_src_ms - buffer_ms;
						//NORMAL_EX_LOG("[%u][--buffer_src_ms  = %u]", i, diff_ms.count());
						// add buffer filter -->
						if (m_buffers_ctx_ptr.size() >= i && !m_stoped)
						{
							
							//if (m_buffers_ctx_ptr[i])
							//if (false)
							{
								ret = ::av_buffersrc_add_frame(m_buffers_ctx_ptr[i], frame_ptr);
								if (ret < 0)
								{
									WARNING_EX_LOG("filter buffer%dsrc add frame failed (%s)!!!\n", i, chen::ffmpeg_util::make_error_string(ret));
									//return ret;
								}
								
							}
						}

					}
					
					::av_frame_unref(frame_ptr);
				}
				if (m_stoped)
				{
					break;
				}
				 if (frame_ptr)
				{
					::av_frame_unref(frame_ptr);
				} 
			
				std::chrono::milliseconds decoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());

				std::chrono::milliseconds diff_ms = decoder_ms - ms;
				//NORMAL_EX_LOG("[%u][decoder_ms = %u]",i,  diff_ms.count());

			}
			//NORMAL_EX_LOG("");
			if (m_stoped)
			{
				//中退出时啦 ^_^
				break;
			}
			//continue;
			std::chrono::milliseconds decoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			std::chrono::milliseconds diff_ms = decoder_ms - ms;
			//NORMAL_EX_LOG("[decoder_ms = %u]", diff_ms.count());
			// get buffersink filer frame --> 
			if ((ret = ::av_buffersink_get_frame(m_buffersink_ctx_ptr, filter_frame_ptr) )<0)
			{
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					//NORMAL_EX_LOG("");
					// 需要继续处理啦
					::av_frame_unref(filter_frame_ptr);
					continue;
				}
				//filter error 
				WARNING_EX_LOG("[video_channel = %s][buffersink get frame = %s]", m_video_split_channel, ffmpeg_util::make_error_string(ret));
				break;
			}
			if (ret < 0)
			{
				// filter 错误啦 ^_^
				WARNING_EX_LOG("video split [name = %s] [filter error = %s] failed !!!", m_video_split_name.c_str(), ffmpeg_util::make_error_string(ret));
				continue;
			}
			//NORMAL_EX_LOG("---> frame -- encoder ");
			// 放到编码器中去编码啦 ^_^
			if (!m_stoped)
			{
				m_encoder_ptr->push_frame(filter_frame_ptr);
			}
			::av_frame_unref(filter_frame_ptr);
			std::chrono::milliseconds encoder_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			  diff_ms = encoder_ms - decoder_ms  ;
			//NORMAL_EX_LOG("[encoder_ms = %u]", diff_ms.count());
		}
		//NORMAL_EX_LOG("");
		::av_frame_free(&filter_frame_ptr);
		if (frame_ptr)
		{
			::av_frame_unref(frame_ptr);
			//::av_frame_free(&frame_ptr);
			frame_ptr = NULL;
		}
		filter_frame_ptr = NULL;
	}
}