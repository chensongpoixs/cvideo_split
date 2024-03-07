/***********************************************************************************************
created: 		2024-01-25

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

#include "cencoder.h"
#include "cffmpeg_util.h"
#include "crect_mem_copy.h"
#include "clog.h"
#include "ccfg.h"
#include <chrono>
namespace chen {




	static int set_hwframe_ctx(AVCodecContext* ctx, AVBufferRef* hw_device_ctx)
	{
		AVBufferRef* hw_frames_ref;
		AVHWFramesContext* frames_ctx = NULL;
		int err = 0;

		if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
			fprintf(stderr, "Failed to create VAAPI frame context.\n");
			return -1;
		}
		frames_ctx = (AVHWFramesContext*)(hw_frames_ref->data);
		frames_ctx->format = AV_PIX_FMT_CUDA;
		frames_ctx->sw_format = AV_PIX_FMT_NV12;
		//frames_ctx->sw_format = AV_PIX_FMT_YUV420P;
		frames_ctx->width = ctx->width;
		frames_ctx->height = ctx->height;
		frames_ctx->initial_pool_size = 20;
		if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
			fprintf(stderr, "Failed to initialize VAAPI frame context."
				"Error code: %s\n", ffmpeg_util::make_error_string(err));
			av_buffer_unref(&hw_frames_ref);
			return err;
		}
		ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
		if (!ctx->hw_frames_ctx)
			err = AVERROR(ENOMEM);

		av_buffer_unref(&hw_frames_ref);
		return err;
	}

	cencoder::~cencoder()
	{
	}
	 bool cencoder::init(uint32 gpu_index,   const char* url,  uint32_t width, uint32_t height)
	{
		{
			destroy();
		}
		{
			std::lock_guard<std::mutex> lock(g_ffmpeg_lock);
			int ret = 0;
			//	if (m_stoped)
			{

			}
			/*m_input_file_ptr = ::fopen(file_url, "rb");
			if (!m_input_file_ptr)
			{
				printf("[warr] not open file name [%s] failed !!!\n", file_url);
				return false;
			}*/

			m_stoped = false;
			m_url = url;
			m_width = width;
			m_height = height;
			m_gpu_index = gpu_index;
			/*m_push_format_context_ptr = avformat_alloc_context();
			if (!m_push_format_context_ptr)
			{
				WARNING_EX_LOG("alloc  avformat context  failed !!! \n");
				return false;
			}*/
			ret = avformat_alloc_output_context2(&m_push_format_context_ptr, NULL, "mpegts", std::string(m_url + "?pkt_size=1316").c_str());
			if (ret < 0)
			{
				WARNING_EX_LOG("avformat alloc output context2  failed !!! [ret = %s]\n", ffmpeg_util::make_error_string(ret));
				return false;
			}
			m_push_format_context_ptr->max_delay = 0;
			m_push_format_context_ptr->max_interleave_delta = 0;
			/*::av_opt_set(m_push_format_context_ptr->priv_data, "MpegTSWrite", "1", 0);
			::av_opt_set(m_push_format_context_ptr->priv_data, "pes_payload_size", "300", 0);*/
			m_stream_ptr = avformat_new_stream(m_push_format_context_ptr, NULL); //分配流空间
			if (!m_stream_ptr)
			{
				WARNING_EX_LOG("[%s] alloc stream failed !!!\n", m_url.c_str());
				return false;
			}
			//m_stream_ptr->codecpar->nb_coded_side_data
			//::avcodec_register_all();
			if (m_push_format_context_ptr->oformat->flags & AVFMT_GLOBALHEADER)
			{
				m_push_format_context_ptr->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}
			m_codec_id = AV_CODEC_ID_H264;

			//AVBufferRef* m_hw_device_ctx = NULL;
			//创建GPU设备 默认第一个设备  也可以指定gpu 索引id 
			//std::string gpu_index = "0";
			ret = av_hwdevice_ctx_create(&m_hw_device_ctx_ptr, AV_HWDEVICE_TYPE_CUDA, std::to_string(m_gpu_index).c_str(), NULL, 0);

			//m_codec_ptr = ::avcodec_find_encoder(m_codec_id);
			m_codec_ptr = ::avcodec_find_encoder_by_name("h264_nvenc");
			if (!m_codec_ptr)
			{
				WARNING_EX_LOG("Could not find [encoder = %s]   failed !!!\n", ::avcodec_get_name(m_codec_id));
				return false;
			}
			m_codec_ctx_ptr = avcodec_alloc_context3(m_codec_ptr);
			if (!m_codec_ctx_ptr)
			{
				WARNING_EX_LOG("Conlu not alloc video context (%s)failed !!!\n", m_url.c_str());
				return false;
			}
			//avctx->width = width;
			//avctx->height = height;
			//avctx->time_base = (AVRational){ 1, 25 };
			//avctx->framerate = (AVRational){ 25, 1 };
			//avctx->sample_aspect_ratio = (AVRational){ 1, 1 };
			//avctx->pix_fmt = AV_PIX_FMT_VAAPI;
			//4000 * 1024;//
			uint64 config_rate = g_cfg.get_uint32(ECI_MediaRate);
			m_codec_ctx_ptr->bit_rate = config_rate * 1000;///*m_width * m_height * 25 * 1*/ 100000;
			//m_codec_ctx_ptr->bit_rate_tolerance = 400000;
			m_codec_ctx_ptr->width = m_width;
			m_codec_ctx_ptr->height = m_height;

			AVRational rate;
			rate.num = 1;
			rate.den = 25;
			m_codec_ctx_ptr->time_base = { 1, 25 };//rate;
			m_codec_ctx_ptr->framerate = { 25, 1 };
			m_codec_ctx_ptr->gop_size = 60;
			m_codec_ctx_ptr->max_b_frames = 0;
			m_codec_ctx_ptr->pix_fmt = AV_PIX_FMT_CUDA;
			//	if (false)
			{
				/* set hw_frames_ctx for encoder's AVCodecContext */
				if ((ret = set_hwframe_ctx(m_codec_ctx_ptr, m_hw_device_ctx_ptr)) < 0) {
					WARNING_EX_LOG("Failed to set hwframe context.\n");
					//goto close;
				}

			}
			//m_codec_ctx_ptr->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			m_pts = AV_TIME_BASE;
			m_frame_count = 0;
			m_mic = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			// 设置GPU的id
			//av_opt_set(m_codec_ctx_ptr->priv_data, "gpu", "1", 0);
			//av_opt_set(m_codec_ctx_ptr->priv_data, "preset", "medium", 0);
			av_opt_set(m_codec_ctx_ptr->priv_data, "preset", "slow", 0);
			//设置零延迟(本地摄像头视频流保存如果不设置则播放的时候会越来越模糊)
			//av_opt_set(m_codec_ctx_ptr->priv_data, "tune", "zerolatency", 0);
			// profile 
			//::av_opt_set(m_codec_ctx_ptr->priv_data, "profile", "baseline", 0);

			::av_opt_set(m_codec_ctx_ptr->priv_data, "buffer_size", "1024", 0);


			//AVDictionary* options= NULL;
			//av_dict_set(&options, "gpu", "1", 0);
			ret = avcodec_open2(m_codec_ctx_ptr, m_codec_ptr,  /*&options*/ NULL);
			if (ret < 0)
			{
				printf("[%s]Open encoder (%s)failed !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return false;
			}

			//av_lockmgr_register();
			ret = avcodec_parameters_from_context(m_push_format_context_ptr->streams[0]->codecpar, m_codec_ctx_ptr);
			if (ret < 0)
			{
				WARNING_EX_LOG("[%s]Failed to copy encoder parameters to output stream #%s\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return false;
			}
			// 设置 mpeg ts page size 包一定要是 1316  在vlc中才能解析
			ret = av_dict_set(&m_options_ptr, "pkt_size", "1316", 0 /*AVIO_FLAG_WRITE*/);
			ret = av_dict_set(&m_options_ptr, "buffer_size", "1024", 0 /*AVIO_FLAG_WRITE*/);
			::av_dict_set(&m_options_ptr, "reuse", "1", 0);

			::av_dump_format(m_push_format_context_ptr, 0, std::string(m_url + "?pkt_size=1316").c_str(), 1);
			const AVDictionaryEntry* e = NULL;
			while (e = av_dict_get(m_options_ptr, "", e, AV_DICT_IGNORE_SUFFIX))
			{
				NORMAL_EX_LOG("[key = %s][value = %s]\n", e->key, e->value);
			}


			//****分配推流io上下文****//
			ret = ::avio_open2(&m_push_format_context_ptr->pb, m_url.c_str(), AVIO_FLAG_WRITE, NULL, &m_options_ptr);
			if (ret < 0)
			{
				WARNING_EX_LOG("Could not open output file '%s' [%s]\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return false;
			}

			//写入头
			ret = avformat_write_header(m_push_format_context_ptr, NULL); //写入头
			if (ret < 0)
			{
				WARNING_EX_LOG("[%s]Error occurred when opening output file [%s]\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return false;
			}
			/*m_frame_ptr = ::av_frame_alloc();
			if (!m_frame_ptr)
			{
				printf("[url = %s] frame alloc failed !!!\n", m_url.c_str());
				return false;
			}
			m_frame_ptr->format = m_codec_ctx_ptr->pix_fmt;
			m_frame_ptr->width = m_codec_ctx_ptr->width;
			m_frame_ptr->height = m_codec_ctx_ptr->height;*/




			//allocate AVFrame data 
			//ret = ::av_image_alloc(m_frame_ptr->data, m_frame_ptr->linesize, m_codec_ctx_ptr->width, m_codec_ctx_ptr->height,
			//	m_codec_ctx_ptr->pix_fmt, 32/*av_cpu_max_align()*/);

			/*if (ret < 0)
			{
				printf("[url = %s]alloc image faild (%s) !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return false;
			}*/
			m_pkt_ptr = av_packet_alloc();
			if (!m_pkt_ptr)
			{
				WARNING_EX_LOG("[url = %s]  alloc pakcet failed !!!\n", m_url.c_str());
				return false;
			}

			return true;
			//AVFrame* hw_frame = NULL;
			return _init_gpu_frame();


		}

	/*	m_yuv420p_ptr = static_cast<unsigned char *>(::malloc(sizeof(unsigned char) * (m_width * m_height * 471 * 2)));
		
		uint32_t frame_size = m_width * m_height * 3 / 2;
		::fread(m_yuv420p_ptr, 1, frame_size * 471, m_input_file_ptr);*/

		//m_thread = std::thread(&cencoder::_work_pthread, this);
		return true;
	}

	void cencoder::destroy()
	{
		std::lock_guard<std::mutex> lock(g_ffmpeg_lock);
		if (!m_stoped)
		{
			WARNING_EX_LOG("[url = %s] stop = false\n", m_url.c_str());
		//	return;
		}
		m_url.clear();

		
		
		if (m_codec_ctx_ptr)
		{
			  if (m_codec_ctx_ptr->hw_device_ctx)
			{
				::av_buffer_unref(&m_codec_ctx_ptr->hw_device_ctx);
			}
			::avcodec_flush_buffers(m_codec_ctx_ptr);  
			::avcodec_close(m_codec_ctx_ptr);
			  ::avcodec_free_context(&m_codec_ctx_ptr);
			m_codec_ctx_ptr = NULL;
		}
		//sws_ctx = nullptr;
		if (m_hw_frame_ptr)
		{
			::av_frame_unref(m_hw_frame_ptr);
			::av_frame_free(&m_hw_frame_ptr);
			m_hw_frame_ptr = NULL;
		}
		 
		if (m_options_ptr)
		{
			av_dict_free(&m_options_ptr);
			m_options_ptr = NULL;
		}
		if (m_pkt_ptr)
		{
			::av_packet_unref(m_pkt_ptr);
			//av_packet_close(m_pkt_ptr);
			::av_packet_free(&m_pkt_ptr);
			m_pkt_ptr = NULL;
		}
		 
		

		if (m_push_format_context_ptr)
		{
			if (m_push_format_context_ptr->pb)
			{
				//::avio_flush(m_push_format_context_ptr->pb);
				//::avio_close(m_push_format_context_ptr->pb);
				//::avio_context_free(&m_push_format_context_ptr->pb);
				//m_push_format_context_ptr->pb = NULL;
			}
			  ::avformat_flush(m_push_format_context_ptr);
			 ::avformat_close_input(&m_push_format_context_ptr); 
			::avformat_free_context(m_push_format_context_ptr);
			m_push_format_context_ptr = NULL;
		}
		m_hw_device_ctx_ptr = NULL;
		m_codec_id = AV_CODEC_ID_NONE;
		m_stoped = true;
		//std::string			m_url;
		//uint32_t			m_width;
		//uint32_t			m_height;
		//AVFormatContext* m_push_format_context_ptr;
		//enum AVCodecID		m_codec_id;
		//AVCodecContext* m_codec_ctx_ptr;
		//const AVCodec* m_codec_ptr;
		//AVStream* m_stream_ptr;
		//AVDictionary* m_options_ptr; // 参数
		//AVPacket* m_pkt_ptr;
		//clock_type			  m_frame_lock;
		//std::list< AVFrame* > m_frame_list;
		//bool				m_stoped;
		//std::thread			m_thread;
		//uint64_t				m_frame_count;
		//uint64_t				m_pts;
		//AVFrame* m_hw_frame_ptr;

	}

	void cencoder::stop()
	{
		m_stoped = true;
	}

	void cencoder::push_frame(  AVFrame* frame_ptr, uint64 dts  , uint64 pts  )
	{
		//return;
		if (m_stoped)
		{
			//::av_frame_unref(frame_ptr);
			return;
		}
		int32_t ret = 0;

		//m_pkt_ptr->data = NULL;
		//m_pkt_ptr->size = 0; 
		++m_frame_count;
		{ 
			if (!m_hw_frame_ptr)
			{
				if (!_init_gpu_frame())
				{
					// init gpu frame failed !!! 
					return;
				}
			}
			if (frame_ptr)
			{
				frame_ptr->pts = m_frame_count;
			}
			if ((ret = ::av_hwframe_transfer_data(m_hw_frame_ptr, frame_ptr, 0)) < 0) 
			{
				 
				WARNING_EX_LOG("Error while transferring frame data to surface."
					"Error code: %s. ", ffmpeg_util::make_error_string(ret));
				return;
			}
			if (m_hw_frame_ptr)
			{
				m_hw_frame_ptr->pts = m_frame_count;
			}
			ret = ::avcodec_send_frame(m_codec_ctx_ptr, m_hw_frame_ptr);
			if (ret < 0)
			{
				//::av_frame_unref(frame_ptr);
				WARNING_EX_LOG("[warr][url = %s] codec send frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return;
			}
		//	av_frame_free(&hw_frame);
		}

		
		//::av_frame_unref(frame_ptr);
		//::av_frame_free(&frame_ptr);
		//frame_ptr = NULL;
		
		//if (ret < 0)
		//{
		//	av_packet_unref(m_pkt_ptr);
		//	WARNING_EX_LOG("[warr][url = %s]codec  receive packet  (%s) failed !!! ", m_url.c_str(), ffmpeg_util::make_error_string(ret));
		//	return;
		//
		//}

		//current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
		//	std::chrono::system_clock::now().time_since_epoch()) - m_mic;
		ret = 0;
		while (ret >= 0)
		{
			ret = ::avcodec_receive_packet(m_codec_ctx_ptr, m_pkt_ptr);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				av_packet_unref(m_pkt_ptr);
				WARNING_EX_LOG("[warr][url = %s]codec  receive packet  (%s) failed !!! ", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return;
			}
			else if (ret < 0)
			{
				av_packet_unref(m_pkt_ptr);
				WARNING_EX_LOG("[warr][url = %s][ret --> ]codec  receive packet  (%s) failed !!! ", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				
				return ;
			}				
			break;
		}
		/*if (dts > pts)
		{
			m_pkt_ptr->pts = dts;
			m_pkt_ptr->dts = dts;
		}
		else 
		{
			m_pkt_ptr->dts = pts;
			m_pkt_ptr->pts = pts;
		}*/
		 

		 //pts (1312130528) < dts (5607097824)
		 //dts (5607097824)
		if (dts > m_pts)
		{
			m_pts = dts;

		}
		else
		{
			m_pts += 4000;
		}
		m_pkt_ptr->pts = m_pts; // std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();// (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.pts;// + (int)(duration*AV_TIME_BASE);

		   m_pkt_ptr->dts = m_pts;// m_pkt_ptr->pts; // (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.dts;// + (int)(duration*AV_TIME_BASE);
		//NORMAL_EX_LOG("pts = %u, dts = %u", m_pkt_ptr->pts, m_pkt_ptr->dts);
		//::av_packet_rescale_ts(m_pkt_ptr, frame_ptr->time_base,  m_stream_ptr->time_base);
		m_pkt_ptr->stream_index = 0;
		//ret = ::av_write_frame(m_push_format_context_ptr, m_pkt_ptr);
		ret = av_interleaved_write_frame(m_push_format_context_ptr, m_pkt_ptr);
		if (ret < 0)
		{
			WARNING_EX_LOG("[error][url = %s] interleaved write frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
		}
		::av_packet_unref(m_pkt_ptr);
 

	}

	void cencoder::consume_frame1(const AVFrame* frame_ptr
	/*const uint8_t* data, int32_t step, int32_t width, uint32_t height, int32_t cn*/ )
	{
		if (m_stoped)
		{
			return;
		}
		/*if ( width != m_frame_ptr->width ||  height != m_frame_ptr->height)
		{
			printf("[warr] width = %u!= encoder width = %u\n",  width, m_frame_ptr->width);
			return;
		}*/

		{
			/*rect_mem_copy(frame_ptr->data[0], frame_ptr->width, frame_ptr->height,
				&m_frame_ptr->data[0], frame_ptr->width * 2, frame_ptr->height,
				0, 0, 0, 0, frame_ptr->width, frame_ptr->height, EFormatYuv420P);
			rect_mem_copy(frame_ptr->data[1], frame_ptr->width / 2, frame_ptr->height / 2,
				&m_frame_ptr->data[1], frame_ptr->width, frame_ptr->height / 2,
				0, 0, 0, 0, frame_ptr->width / 2, frame_ptr->height / 2, EFormatYuv420P);

			rect_mem_copy(frame_ptr->data[2], frame_ptr->width / 2, frame_ptr->height / 2,
				&m_frame_ptr->data[2], frame_ptr->width, frame_ptr->height / 2,
				0, 0, 0, 0, frame_ptr->width / 2, frame_ptr->height / 2, EFormatYuv420P);*/
			/*rect_mem_copy(data, width, height,
				&m_frame_ptr->data[0], width * 2,  height  , 
				0, 0, 0, 0, width , height , EFormatYuv420P);*/

			 
			 /*rect_mem_copy(data + (width * height), width/2, height/2,
				&m_frame_ptr->data[1], width, height/2, 
				0, 0, 0, 0, width /2 , height /2 , EFormatYuv420P);

			rect_mem_copy(data + ((width * height) * 5/4), width/2, height/2,
				&m_frame_ptr->data[2], width , height/2 , 
				0, 0, 0, 0, width/2, height/2, EFormatYuv420P); */

			 
		}
		/*memcpy(m_frame_ptr->data[0], data,  width *  height);
		memcpy(m_frame_ptr->data[1], data + (width * height),  width *  height / 4);
		memcpy(m_frame_ptr->data[2], data + ((width * height) *5/4), width * height / 4);*/
	//	memcpy(m_frame_ptr->data[2], frame_ptr->data[2], frame_ptr->width * frame_ptr->height / 4);

		//int ret = av_frame_copy(m_frame_ptr, frame);
		//if (ret < 0)
		//{
		//	printf("copy frame (%s)failed !!!\n", ffmpeg_util::make_error_string(ret));
		//}
	}

	void cencoder::consume_frame2(const AVFrame* frame_ptr
		/*const uint8_t* data, int32_t step, int32_t width, uint32_t height, int32_t cn*/)
	{
		 
		if (m_stoped)
		{
			return;
		}
		{
			/*rect_mem_copy(frame_ptr->data[0], frame_ptr->width, frame_ptr->height,
				&m_frame_ptr->data[0], frame_ptr->width * 2, frame_ptr->height,
				0, 0,
				frame_ptr->width, 0, frame_ptr->width, frame_ptr->height, EFormatYuv420P);
			  rect_mem_copy(frame_ptr->data[1], frame_ptr->width / 2, frame_ptr->height / 2,
				&m_frame_ptr->data[1], frame_ptr->width, frame_ptr->height / 2,
				0, 0,
				  frame_ptr->width/2, 0, frame_ptr->width / 2, frame_ptr->height / 2, EFormatYuv420P);

				 rect_mem_copy(frame_ptr->data[2], frame_ptr->width / 2, frame_ptr->height / 2,
					&m_frame_ptr->data[2], frame_ptr->width, frame_ptr->height / 2,
					0, 0,
					 frame_ptr->width/2, 0, frame_ptr->width / 2, frame_ptr->height / 2, EFormatYuv420P);*/
			/*rect_mem_copy(data, width, height,
				&m_frame_ptr->data[0], width * 2, height,
				0, 0, 
				width, 0, width, height, EFormatYuv420P);*/


			/* rect_mem_copy(data + (width * height), width / 2, height / 2,
				&m_frame_ptr->data[1], width, height / 2,
				0, 0, 
				width/2, 0, width / 2, height / 2, EFormatYuv420P);*/

			/*rect_mem_copy(data + ((width * height) * 5 / 4), width / 2, height / 2,
				&m_frame_ptr->data[2], width, height / 2,
				0, 0, 
				width/2, 0, width / 2, height / 2, EFormatYuv420P); */

		}
		/*if (width != m_frame_ptr->width || height != m_frame_ptr->height)
		{
			printf("[warr] width = %u!= encoder width = %u\n", width, m_frame_ptr->width);
			return;
		}*/
		/*memcpy(m_frame_ptr->data[0], data, width * height);
		memcpy(m_frame_ptr->data[1], data + (width * height), width * height / 4);
		memcpy(m_frame_ptr->data[2], data + ((width * height) * 5 / 4), width * height / 4);*/
		//	memcpy(m_frame_ptr->data[2], frame_ptr->data[2], frame_ptr->width * frame_ptr->height / 4);

			//int ret = av_frame_copy(m_frame_ptr, frame);
			//if (ret < 0)
			//{
			//	printf("copy frame (%s)failed !!!\n", ffmpeg_util::make_error_string(ret));
			//}
	}
 
	void cencoder::_work_pthread()
	{
		uint64_t  frame_count = 0;
		int32_t ret = 0;
		uint32_t frame_size = m_width * m_height * 3 / 2;
		uint32_t index = 0;
		AVRational rate;
		rate.num = 1;
		rate.den = 25;
		time_t cur_time = ::time(NULL);
		uint32_t frame_cycle = 0;
		uint64_t pts = 0;
		// 重新打时间戳
		std::chrono::microseconds mic = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		
		int32_t sleep_ms = 1000  / 30;
		AVFrame* frame_ptr = NULL;
		int32_t frame_num = 0;
		while (!m_stoped)
		{
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			{
				clock_guard lock(m_frame_lock);
				frame_num = m_frame_list.size();
				if (frame_num > 0)
				{
					frame_ptr = m_frame_list.front();
					m_frame_list.pop_front();
				}
				frame_num = m_frame_list.size();
			}
			
			if (frame_ptr)
			{
				
				++frame_cycle;
				if (frame_cycle > 25)
				{
					frame_cycle = 0;
					pts = 0;
				}
				m_pkt_ptr->data = NULL;
				m_pkt_ptr->size = 0;
				pts += (100000 / 120);
				std::chrono::microseconds current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::system_clock::now().time_since_epoch()) - mic;
				frame_ptr->pts = (current_mic.count() / 10) + AV_TIME_BASE;
				++frame_count;
				if (frame_count == 500)
				{
					frame_count = 0;
				}
				ret = ::avcodec_send_frame(m_codec_ctx_ptr, frame_ptr);
				if (ret < 0)
				{
					WARNING_EX_LOG("[warr][url = %s] codec send frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
					continue;
				}
				::av_frame_free(&frame_ptr);
				frame_ptr = NULL;
				ret = ::avcodec_receive_packet(m_codec_ctx_ptr, m_pkt_ptr);
				if (ret < 0)
				{
					WARNING_EX_LOG("[warr][url = %s]codec  receive packet  (%s) failed !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
					continue;

				}

				current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::system_clock::now().time_since_epoch()) - mic;
				m_pkt_ptr->pts = (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.pts;// + (int)(duration*AV_TIME_BASE);

				m_pkt_ptr->dts = (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.dts;// + (int)(duration*AV_TIME_BASE);

				m_pkt_ptr->stream_index = 0;
				//NORMAL_EX_LOG(" [frame = %lu][%lu]  [pts = %lu]\n", frame_count, ::time(NULL), m_pkt_ptr->dts);

				//1706158630000000 >= 1706158630000000

				//printf("pts:%d , dts:%d , duration*AV_TIME_BASE:%d\n", encodePacket.pts, encodePacket.dts, (int)(duration * AV_TIME_BASE));

				//av_packet_rescale_ts(m_pkt_ptr, pInStream->time_base, pOutStream->time_base);

				//m_pkt_ptr->pos = -1;
				//printf("[frame = %u][dts = %u]\n", frame_count, m_pkt_ptr->dts);
				// 4. 将编码后的packet写入输出媒体文件
				ret = av_interleaved_write_frame(m_push_format_context_ptr, m_pkt_ptr);
				if (ret < 0)
				{
					WARNING_EX_LOG("[error][url = %s] interleaved write frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				}
			}
			if (frame_num <= 0)
			{
				std::chrono::milliseconds cur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
				std::chrono::milliseconds diff_ms = cur_ms - ms;
				ms = cur_ms;

				if (sleep_ms > diff_ms.count())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms - diff_ms.count()));
				}
			}
			//std::this_thread::sleep_for(std::chrono::microseconds(10000000/25));
		}
	}

	bool cencoder::_init_gpu_frame()
	{
		if (m_hw_frame_ptr)
		{
			// warring hw gpu frame !!!
			return true;
		}
		int32_t ret = 0;
		m_hw_frame_ptr = ::av_frame_alloc();
		if (!m_hw_frame_ptr)
		{
			printf("alloc gpu hw frame  failed !!!\n");
			return false;
		}
		/*if (!(m_hw_frame = av_frame_alloc())) {
			ret = AVERROR(ENOMEM);
			return;
		}*/
		ret = av_hwframe_get_buffer(m_codec_ctx_ptr->hw_frames_ctx, m_hw_frame_ptr, 0);
		if (ret < 0) {
			printf("Error code: %s.\n", ffmpeg_util::make_error_string(ret));
			return false;
		}
		if (!m_hw_frame_ptr->hw_frames_ctx)
		{
			printf("[gpu hw frame ctx failed !!!]\n");
			//	ret = AVERROR(ENOMEM);
			return false;
		}
		return true;
	}

}
