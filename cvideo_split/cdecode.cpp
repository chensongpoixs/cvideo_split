/***********************************************************************************************
created: 		2023-11-18

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

#include "cdecode.h"
#include "cffmpeg_util.h"
#include <cassert>
#include "ccfg.h"
#include "clog.h"

namespace chen {

	/*static char g_errorbuffer[AV_ERROR_MAX_STRING_SIZE];
	char* ffmepgerror(int errCode)
	{
		
		return av_make_error_string(g_errorbuffer, AV_ERROR_MAX_STRING_SIZE, errCode);
	}*/

	static AVPixelFormat get_format(AVCodecContext* avctx, const enum AVPixelFormat* pix_fmts)
	{
		while (*pix_fmts != AV_PIX_FMT_NONE) {
			if (*pix_fmts == AV_PIX_FMT_CUDA) {
				return AV_PIX_FMT_CUDA;
			}

			pix_fmts++;
		}

		WARNING_EX_LOG( "The CUDA pixel format not offered in get_format()\n");

		return AV_PIX_FMT_NONE;
	}

	static int set_hwframe_ctx(AVCodecContext* ctx, AVBufferRef* hw_device_ctx)
	{
		AVBufferRef* hw_frames_ref;
		AVHWFramesContext* frames_ctx = NULL;
		int err = 0;

		if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
			WARNING_EX_LOG( "Failed to create VAAPI frame context.\n");
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
	cdecode::~cdecode()
	{
	}
	bool cdecode::init(uint32 gpu_index, const char* url , uint32 index, cvideo_splist* ptr)
	{
		
		std::lock_guard<std::mutex> lock(g_ffmpeg_lock);
		{
			close();
		}
		m_reconnect = 0;
		m_gpu_index = gpu_index;
		m_video_stream_ptr = NULL;
		m_open = false;
		m_url = url;
#if USE_AV_INTERRUPT_CALLBACK
 
		m_open_timeout = g_cfg.get_uint32(ECI_MediaOpenTimeOut); //LIBAVFORMAT_INTERRUPT_OPEN_DEFAULT_TIMEOUT_MS;
		m_read_timeout = g_cfg.get_uint32(ECI_MediaReadTimeOut);//LIBAVFORMAT_INTERRUPT_READ_DEFAULT_TIMEOUT_MS;
 
		/* interrupt callback */
		m_interrupt_metadata.timeout_after_ms = m_open_timeout;
		get_monotonic_time(&m_interrupt_metadata.value);

		m_ic_ptr = avformat_alloc_context();
		m_ic_ptr->interrupt_callback.callback = &ffmpeg_util:: ffmpeg_interrupt_callback;
		m_ic_ptr->interrupt_callback.opaque = &m_interrupt_metadata;
#endif
		//char* options = getenv("OPENCV_FFMPEG_CAPTURE_OPTIONS");
#if LIBAVUTIL_BUILD >= (LIBAVUTIL_VERSION_MICRO >= 100 ? CALC_FFMPEG_VERSION(52, 17, 100) : CALC_FFMPEG_VERSION(52, 7, 0))
		//av_dict_parse_string(&m_dict, options, ";", "|", 0);
#else
		//av_dict_set(&m_dict, "rtsp_transport", "tcp", 0);
#endif
		m_picture_pts = AV_NOPTS_VALUE;
		memset(&m_frame, 0, sizeof(m_frame));
		m_frame_number = 0;
		m_first_frame_number = -1;
		m_eps_zero = 0.000025;

		m_rotation_angle = 0;

#if (LIBAVUTIL_BUILD >= CALC_FFMPEG_VERSION(52, 92, 100))
		m_rotation_auto = true;
#else
		m_rotation_auto = false;
#endif
		//花屏问题
		// cat /proc/sys/net/core/rmem_max 对应缓存大小  212992*2 = 425984   ==> 425984
		// echo 10240000 > /proc/sys/net/core/rmem_max
		::av_dict_set(&m_dict, "buffer_size", "10240000", 0);
		::av_dict_set(&m_dict, "reuse", "1", 0);
		if (g_cfg.get_uint32(ECI_UdpRecvBufferEnable) > 0)
		{
			
			// "udp://@239.1.1.7:5107?overrun_nonfatal=1&fifo_size=50000000"
			::av_dict_set(&m_dict, "fifo_size", "50000000", 0);
		}
		if (g_cfg.get_uint32(ECI_UdpRecvBufferOverrunNonfatal) > 0)
		{
			::av_dict_set(&m_dict, "overrun_nonfatal", "1", 0);
		}
		const AVInputFormat* input_format = NULL;
		AVDictionaryEntry* entry = av_dict_get(m_dict, "input_format", NULL, 0);
		if (entry != 0)
		{
			input_format = av_find_input_format(entry->value);
		}

		// 1. 打开解封装上下文
		int ret = avformat_open_input(
			&m_ic_ptr, //解封装上下文
			url,  //文件路径
			input_format, //指定输入格式 h264,h265, 之类的， 传入NULL则自动检测
			&m_dict); //设置参数的字典
		if (ret != 0)
		{
			WARNING_EX_LOG("%s\n", ffmpeg_util::make_error_string(ret));
			return false;
		}
		//2.读取文件信息
		ret = avformat_find_stream_info(m_ic_ptr, NULL);
		if (ret < 0)
		{
			WARNING_EX_LOG("%s\n", ffmpeg_util::make_error_string(ret));
			return false;
		}
		//3.获取目标流索引
		for (unsigned int i = 0; i < m_ic_ptr->nb_streams; i++)
		{
			AVStream* stream = m_ic_ptr->streams[i];
			if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				if (!m_video_stream_ptr)
				{
					m_video_stream_index = i;
					m_video_stream_ptr = stream;
				}
				else
				{
					stream->discard = AVDISCARD_ALL;
				}
			}
			else
			{
				stream->discard = AVDISCARD_ALL;
			}
		}
		const AVCodec* codec = NULL;
		//4.查找解码器
		if (AV_CODEC_ID_H264 == m_video_stream_ptr->codecpar->codec_id)
		{
			codec = ::avcodec_find_decoder_by_name("h264_cuvid");
		}
		else if (AV_CODEC_ID_HEVC == m_video_stream_ptr->codecpar->codec_id)
		{
			codec = ::avcodec_find_decoder_by_name("hevc_cuvid");
		}
		else
		{
			codec = ::avcodec_find_decoder(m_video_stream_ptr->codecpar->codec_id);
		}
		if (!codec)
		{
			WARNING_EX_LOG("can't find codec, codec id:%d ", m_video_stream_ptr->codecpar->codec_id);
			return false;
		}
		//AVBufferRef* hw_device_ctx = NULL;
		////创建GPU设备 默认第一个设备  也可以指定gpu 索引id 
		//std::string gpu_index = "0";
		//ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, gpu_index.c_str(), NULL, 0);

		//5.创建解码器上下文
		if (!(m_codec_ctx_ptr = avcodec_alloc_context3(codec)))
		{
			WARNING_EX_LOG("avcodec_alloc_context3 failed !!! ");
			return false;
		}

		//6.从输入流复制编解码器参数到输出编解码器上下文
		if ((ret = avcodec_parameters_to_context(m_codec_ctx_ptr, m_video_stream_ptr->codecpar)) < 0)
		{
			WARNING_EX_LOG("Failed to copy %s codec parameters to decoder context ",
				av_get_media_type_string(m_video_stream_ptr->codecpar->codec_type));
			return false;
		}
		{
			/* set hw_frames_ctx for encoder's AVCodecContext */
			//if ((ret = set_hwframe_ctx(m_codec_ctx_ptr, hw_device_ctx)) < 0) {
			//	WARNING_EX_LOG("Failed to set hwframe context.\n");
			//	//goto close;
			//}
			//m_codec_ctx_ptr->hw_device_ctx = av_buffer_ref(hw_device_ctx);
			//m_codec_ctx_ptr->get_format = get_format;
		}
		AVDictionary* codec_opts = NULL;
		//av_dict_set(&codec_opts, "b", "2.5M", 0);
		av_dict_set(&codec_opts, "gpu", std::to_string(m_gpu_index).c_str(), 0);
		av_dict_set(&codec_opts, "threads", "auto", 0);
		av_dict_set(&codec_opts, "flags", "+copy_opaque", AV_DICT_MULTIKEY);
	/*	name dirtc[key = gpu][value = 1]
			name dirtc[key = threads][value = auto]
			name dirtc[key = flags][value = +copy_opaque]*/
		NORMAL_EX_LOG("use gpu index = %u", m_gpu_index);
		//7. 打开解码器上下文 */
		if ((ret = avcodec_open2(m_codec_ctx_ptr, codec, &codec_opts)) < 0)
		{
			::av_dict_free(&codec_opts);
			WARNING_EX_LOG("Failed to open %s [gpu = %u] codec (%s)",
				av_get_media_type_string(m_video_stream_ptr->codecpar->codec_type), m_gpu_index, ffmpeg_util::make_error_string(ret));
			return false;
		}
		::av_dict_free(&codec_opts);
		//创建一个frame接收解码之后的帧数据
		m_picture_ptr = av_frame_alloc();
		m_packet_ptr = av_packet_alloc();
		m_open = true;
		m_width = m_video_stream_ptr->codecpar->width;
		m_height = m_video_stream_ptr->codecpar->height;
		m_frame.width = m_codec_ctx_ptr->width;
		m_frame.height = m_codec_ctx_ptr->height;
		m_frame.cn = 3;
		m_frame.step = 0;
		m_frame.data = NULL;
		//检测是否支持当前视频的像素格式
		m_pixfmt = (AVPixelFormat)m_video_stream_ptr->codecpar->format;
		 
#if USE_AV_INTERRUPT_CALLBACK
		// deactivate interrupt callback
		m_interrupt_metadata.timeout_after_ms = 0;
#endif
		get_rotation_angle();
		return true;
		//return false;
	}
	void cdecode::destroy()
	{
		std::lock_guard<std::mutex> lock(g_ffmpeg_lock);
		close();

	}
	void cdecode::close()
	{

#if USE_AV_INTERRUPT_CALLBACK
		// deactivate interrupt callback
		m_interrupt_metadata.timeout_after_ms = 0;
#endif
		 
		m_stoped = true;
		
		
		if (m_sws_ctx_ptr)
		{
			::sws_freeContext(m_sws_ctx_ptr);
			m_sws_ctx_ptr = NULL;
		}
		
		if (m_codec_ctx_ptr)
		{
			  if (m_codec_ctx_ptr->hw_device_ctx)
			{
				av_buffer_unref(&m_codec_ctx_ptr->hw_device_ctx);
			}
			::avcodec_flush_buffers(m_codec_ctx_ptr);  
			::avcodec_close(m_codec_ctx_ptr);
			::avcodec_free_context(&m_codec_ctx_ptr);
			m_codec_ctx_ptr = NULL;
		}
		//sws_ctx = nullptr;
		if (m_picture_ptr)
		{
			::av_frame_free(&m_picture_ptr);
			m_picture_ptr = NULL;
		} 
		if (m_sws_frame_ptr)
		{
			::av_frame_free(&m_sws_frame_ptr);
			m_sws_frame_ptr = NULL;
		}
		if (m_dict)
		{
			av_dict_free(&m_dict);
			m_dict = NULL;
		}
		if (m_packet_ptr)
		{
			av_packet_free(&m_packet_ptr);
			m_packet_ptr = NULL;
		}
		if (m_ic_ptr)
		{
			 ::avformat_flush(m_ic_ptr);
			  ::avformat_close_input(&m_ic_ptr); 
			::avformat_free_context(m_ic_ptr);
			m_ic_ptr = NULL;
		}
		m_stoped = true;
		m_open = false;
		m_pixfmt = AV_PIX_FMT_NONE;
		m_vec_dts.clear();
		m_vec_pts.clear();
		m_reconnect = 0;
	}
	uint64 cdecode::get_index_pts(  uint32 number_frame)
	{
		if (m_vec_dts.size() == 0)
		{
			return 0;
		}
		if (number_frame >= m_vec_pts.size())
		{
			return m_vec_pts[m_vec_pts.size() - 1];
		}
		return m_vec_pts[number_frame];
	}
	uint64 cdecode::get_index_dts(uint32 number_frame)
	{
		if (m_vec_dts.size() == 0)
		{
			return 0;
		}
		if (number_frame >= m_vec_dts.size())
		{

			return m_vec_dts[m_vec_dts.size() - 1];
		}
		return m_vec_dts[number_frame];
	}
	bool cdecode::grab_frame( )
	{
		if (!m_open)
		{
			return false;
		}

		if (!m_ic_ptr || !m_video_stream_ptr || !m_codec_ctx_ptr)
		{
			return false;
		}
		// TODO@chensong 2024-01-26 视频帧数量判断是否最新的新的
		if (m_ic_ptr->streams[m_video_stream_index]->nb_frames > 0 
			&& m_frame_number > m_ic_ptr->streams[m_video_stream_index]->nb_frames)
		{
			return false;
		}
		av_frame_unref(m_picture_ptr);
		const int max_number_of_attempts = 1 << 9;
		int32_t ret = 0;
		int32_t count_errs = 0;
		bool    valid = false;
#if USE_AV_INTERRUPT_CALLBACK
		// activate interrupt callback
		get_monotonic_time(&m_interrupt_metadata.value);
		m_interrupt_metadata.timeout_after_ms = m_read_timeout;
#endif
		//定义AVPacket用来存储压缩的帧数据
		//AVPacket* pkt = av_packet_alloc();;
#if USE_AV_SEND_FRAME_API
		// check if we can receive frame from previously decoded packet
		valid = avcodec_receive_frame(m_codec_ctx_ptr, m_picture_ptr) >= 0;
#endif
		 
		while (!valid)
		{


			av_packet_unref(m_packet_ptr);

#if USE_AV_INTERRUPT_CALLBACK
			if (m_interrupt_metadata.timeout)
			{
				valid = false;
				break;
			}
#endif 
			//读取一帧压缩数据
			ret = av_read_frame(m_ic_ptr, m_packet_ptr);
			if (m_packet_ptr->stream_index != m_video_stream_index   )
			{
				av_packet_unref(m_packet_ptr);
				continue;
			}
			// AVERROR(EIO)
			
			if (ret == AVERROR(EAGAIN))
			{
				av_packet_unref(m_packet_ptr);
				continue;
			}
			if (ret == AVERROR(EIO))
			{
				WARNING_EX_LOG("[url = %s] read packet AVERROR(EIO)  failed !!!", m_url.c_str());
				av_packet_unref(m_packet_ptr);
				m_reconnect = 1000;
				valid = false;
				break;
			}
			if (ret == AVERROR_EOF)
			{
			
				av_packet_unref(m_packet_ptr);
				// TODO@chensong 2024-10-21  崩溃到解码器中 avci-> frame 
				// 
				//::avcodec_flush_buffers(m_codec_ctx_ptr);
				
				++m_reconnect;
				
				valid = false;
				break;
				
			}
			else if (ret < 0)
			{
				av_packet_unref(m_packet_ptr);
				continue;
			}

			/*if (m_packet_ptr->stream_index != m_video_stream_index)
			{
				av_packet_unref(m_packet_ptr);
				count_errs++;
				if (count_errs > max_number_of_attempts)
				{
					break;
				}
				continue;
			}*/
			 if (ret >= 0)
			{
				m_pts = m_packet_ptr->pts;
				m_dts = m_packet_ptr->dts;
			} 
			// Decode video frame
#if USE_AV_SEND_FRAME_API
			if (avcodec_send_packet(m_codec_ctx_ptr, m_packet_ptr) < 0)
			{
				av_packet_unref(m_packet_ptr);
				continue;
			}

			av_packet_unref(m_packet_ptr);
			ret = avcodec_receive_frame(m_codec_ctx_ptr, m_picture_ptr);
#else
			int got_picture = 0;
			avcodec_decode_video2(context, picture, &got_picture, &packet);
			ret = got_picture ? 0 : -1;
#endif

			if (ret >= 0)
			{
				//picture_pts = picture->best_effort_timestamp;
				if (m_picture_pts == AV_NOPTS_VALUE)
				{
					m_picture_pts = m_picture_ptr->CV_FFMPEG_PTS_FIELD != AV_NOPTS_VALUE 
						&& m_picture_ptr->CV_FFMPEG_PTS_FIELD != 0
						? m_picture_ptr->CV_FFMPEG_PTS_FIELD : m_picture_ptr->pkt_dts;
				}

				valid = true;
			}
			else if (ret == AVERROR(EAGAIN)) 
			{
				av_frame_unref(m_picture_ptr);
				continue;
			}
			else
			{
				av_frame_unref(m_picture_ptr);
				count_errs++;
				if (count_errs > max_number_of_attempts)
				{
					break;
				}
			}

		}  

		if (valid)
		{
		//	m_vec_pts.push_back(m_pts);
		//	m_vec_dts.push_back(m_dts);
			++m_frame_number;
			m_reconnect = 0;
		}
		else
		{
			av_frame_unref(m_picture_ptr);
		}
		if (valid && m_first_frame_number < 0)
		{
			m_first_frame_number = dts_to_frame_number(m_picture_pts);
		}


		return valid;
	}
	bool cdecode::retrieve(AVFrame*& frame
		 /*unsigned char** data, int* step, int* width,
		int* height, int* cn*/ )
	{
		//AVFrame* srcFrame = nullptr;
		if (m_picture_ptr)
		{
			av_frame_unref(m_picture_ptr);
		}
		
		bool ret = grab_frame( );
		if (ret )
		{
			 frame = m_picture_ptr;
			 
			/*memcpy(*data, m_picture_ptr->data[0], m_picture_ptr->height * m_picture_ptr->width);
			memcpy((*data) +(m_picture_ptr->height * m_picture_ptr->width) , m_picture_ptr->data[1], m_picture_ptr->height * m_picture_ptr->width/4);
			memcpy((*data) + (m_picture_ptr->height * m_picture_ptr->width *5 /4), m_picture_ptr->data[2], m_picture_ptr->height * m_picture_ptr->width/4);
		*/	/**width = m_picture_ptr->width;
			*height = m_picture_ptr->height;
			*step = m_picture_ptr->linesize[0];
			*cn = m_picture_ptr->ch_layout.nb_channels;*/

#if 0
			AVFrame* sw_picture = m_picture_ptr;
#if USE_AV_HW_CODECS
			// if hardware frame, copy it to system memory
			if (m_picture_ptr && m_picture_ptr->hw_frames_ctx)
			{
				sw_picture = av_frame_alloc();
				//if (av_hwframe_map(sw_picture, picture, AV_HWFRAME_MAP_READ) < 0) {
				if (av_hwframe_transfer_data(sw_picture, m_picture_ptr, 0) < 0)
				{
					printf("Error copying data from GPU to CPU (av_hwframe_transfer_data \n");
					//CV_LOG_ERROR(NULL, "Error copying data from GPU to CPU (av_hwframe_transfer_data)");
					return false;
				}
			}
#endif
			if (!sw_picture || !sw_picture->data[0])
			{
				return false;
			}
			if (m_sws_ctx_ptr == NULL ||
				m_frame.width != m_video_stream_ptr->CV_FFMPEG_CODEC_FIELD->width ||
				m_frame.height != m_video_stream_ptr->CV_FFMPEG_CODEC_FIELD->height ||
				m_frame.data == NULL)
			{
				// Some sws_scale optimizations have some assumptions about alignment of data/step/width/height
	   // Also we use coded_width/height to workaround problem with legacy ffmpeg versions (like n0.8)
				int32_t buffer_width = m_codec_ctx_ptr->coded_width;
				int32_t buffer_height = m_codec_ctx_ptr->coded_height;

				m_sws_ctx_ptr = sws_getCachedContext(
					m_sws_ctx_ptr,
					buffer_width, buffer_height,
					(AVPixelFormat)sw_picture->format,
					buffer_width, buffer_height,
					AV_PIX_FMT_YUV420P,
					SWS_BICUBIC,
					NULL, NULL, NULL
				);

				if (m_sws_ctx_ptr == NULL)
				{
					printf("Cannot initialize the conversion context!!!\n");
					return false;//CV_Error(0, "Cannot initialize the conversion context!");
				}
#if USE_AV_FRAME_GET_BUFFER
				av_frame_unref(m_sws_frame_ptr);
				m_sws_frame_ptr->format = AV_PIX_FMT_YUV420P;
				m_sws_frame_ptr->width = buffer_width;
				m_sws_frame_ptr->height = buffer_height;
				if (0 != av_frame_get_buffer(m_sws_frame_ptr, 32))
				{
					printf("av_frame_get_buffer OutOfMemory \n ");
					return false;
				}
#else
				int aligns[AV_NUM_DATA_POINTERS];
				avcodec_align_dimensions2(video_st->codec, &buffer_width, &buffer_height, aligns);
				rgb_picture.data[0] = (uint8_t*)realloc(rgb_picture.data[0],
					_opencv_ffmpeg_av_image_get_buffer_size(AV_PIX_FMT_BGR24,
						buffer_width, buffer_height));
				_opencv_ffmpeg_av_image_fill_arrays(&rgb_picture, rgb_picture.data[0],
					AV_PIX_FMT_BGR24, buffer_width, buffer_height);
#endif
				m_frame.width = m_video_stream_ptr->CV_FFMPEG_CODEC_FIELD->width;
				m_frame.height = m_video_stream_ptr->CV_FFMPEG_CODEC_FIELD->height;
				m_frame.cn = 3;
				m_frame.data = m_sws_frame_ptr->data[0];
				m_frame.step = m_sws_frame_ptr->linesize[0];
				}
			sws_scale(
				m_sws_ctx_ptr,
				sw_picture->data,
				sw_picture->linesize,
				0, sw_picture->height,
				m_sws_frame_ptr->data,
				m_sws_frame_ptr->linesize
			);

			*data = m_frame.data;
			*step = m_frame.step;
			*width = m_frame.width;
			*height = m_frame.height;
			*cn = m_frame.cn;

#if USE_AV_HW_CODECS
			if (sw_picture != m_picture_ptr)
			{
				av_frame_free(&sw_picture);
			}
#endif
#endif 
			return true;
		}
		 
		return ret;
	}
	bool cdecode::seek(double sec)
	{
		if (!m_open)
		{
			return false;
		}
		if (!m_ic_ptr)
		{
			return false;
		}
		seek((int64_t)(sec * get_fps() + 0.5));
		/*int64_t ts = m_ic_ptr->duration * percentage;
		int ret = ::av_seek_frame(m_ic_ptr, -1, ts, AVSEEK_FLAG_FRAME);
		if (ret < 0)
		{
			printf("Seek error: %s", ffmpeg_util::make_error_string(ret));
			return false;
		}
		::avcodec_flush_buffers(m_codec_ctx_ptr);*/
		return true;
	}
	void cdecode::seek(int64_t frame_number)
	{
		assert(m_ic_ptr);
		m_frame_number = 0;// std::min(m_frame_number, get_total_frames());
		int delta = 16;

		// if we have not grabbed a single frame before first seek, let's read the first frame
		// and get some valuable information during the process
		if (m_first_frame_number < 0 && get_total_frames() > 1)
		{
			grab_frame();
		}

		for (;;)
		{

			int64_t _frame_number_temp = 0;// std::max(m_frame_number - delta, (int64_t)0);
			double sec = (double)_frame_number_temp / get_fps();
			int64_t time_stamp = m_ic_ptr->streams[m_video_stream_index]->start_time;
			double  time_base = r2d(m_ic_ptr->streams[m_video_stream_index]->time_base);
			time_stamp += (int64_t)(sec / time_base + 0.5);
			if (get_total_frames() > 1) 
			{
				av_seek_frame(m_ic_ptr, m_video_stream_index, time_stamp, AVSEEK_FLAG_BACKWARD);
			}
			avcodec_flush_buffers(m_codec_ctx_ptr);
			if (m_frame_number > 0)
			{
				grab_frame();

				if (m_frame_number > 1)
				{
					frame_number = dts_to_frame_number(m_picture_pts) - m_first_frame_number;
					//printf("_frame_number = %d, frame_number = %d, delta = %d\n",
					//       (int)_frame_number, (int)frame_number, delta);

					if (frame_number < 0 || frame_number > m_frame_number - 1)
					{
						if (_frame_number_temp == 0 || delta >= INT_MAX / 4)
						{
							break;
						}
						delta = delta < 16 ? delta * 2 : delta * 3 / 2;
						continue;
					}
					while (frame_number < m_frame_number - 1)
					{
						if (!grab_frame())
						{
							break;
						}
					}
					frame_number++;
					break;
				}
				else
				{
					frame_number = 1;
					break;
				}
			}
			else
			{
				frame_number = 0;
				break;
			}
		}
	}
	int64_t cdecode::get_total_frames() const
	{
		if (!m_ic_ptr)
		{
			return 0;
		}
		int64_t nbf = m_ic_ptr->streams[m_video_stream_index]->nb_frames;

		if (nbf == 0)
		{
			nbf = (int64_t)::floor(get_duration_sec() * get_fps() + 0.5);
		}
		return nbf;
	}
	double cdecode::get_duration_sec() const
	{
		if (!m_ic_ptr)
		{
			return 0.0;
		}
		double sec = (double)m_ic_ptr->duration / (double)AV_TIME_BASE;

		if (sec < m_eps_zero)
		{
			sec = (double)m_ic_ptr->streams[m_video_stream_index]->duration * r2d(m_ic_ptr->streams[m_video_stream_index]->time_base);
		}

		return sec;
	}
	double cdecode::get_fps() const
	{
#if 0 && LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(55, 1, 100) && LIBAVFORMAT_VERSION_MICRO >= 100
		double fps = r2d(av_guess_frame_rate(ic, ic->streams[video_stream], NULL));
#else
		double fps = r2d(m_ic_ptr->streams[m_video_stream_index]->avg_frame_rate);

#if LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(52, 111, 0)
		if (fps < m_eps_zero)
		{
			fps = r2d(m_ic_ptr->streams[m_video_stream_index]->avg_frame_rate);
		}
#endif

		if (fps < m_eps_zero)
		{
			fps = 1.0 / r2d(m_ic_ptr->streams[m_video_stream_index]->time_base);
		}
#endif
		return fps;
	}
	int64_t cdecode::get_bitrate() const
	{
		/*if (!m_ic_ptr)
		{
			return 0;
		}*/
		assert(m_ic_ptr);
		return m_ic_ptr->bit_rate / 1000;
	}
	double cdecode::r2d(AVRational r) const
	{
		return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
	}
	int64_t cdecode::dts_to_frame_number(int64_t dts)
	{
		double sec = dts_to_sec(dts);
		return (int64_t)(get_fps() * sec + 0.5);
	}
	double cdecode::dts_to_sec(int64_t dts) const
	{
		return (double)(dts - m_ic_ptr->streams[m_video_stream_index]->start_time) *
			r2d(m_ic_ptr->streams[m_video_stream_index]->time_base);
	}
	void cdecode::get_rotation_angle()
	{
		m_rotation_angle = 0;
#if LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(57, 68, 100)
		//const uint8_t* data = NULL;
		//av_frame_get_side_data();
		/*const uint8_t* data   = ::av_stream_get_side_data(m_video_stream_ptr, AV_PKT_DATA_DISPLAYMATRIX, NULL);
		if (data)
		{
			m_rotation_angle = -ffmpeg_util::round(av_display_rotation_get((const int32_t*)data));
			if (m_rotation_angle < 0)
			{
				m_rotation_angle += 360;
			}
		} */
#elif LIBAVUTIL_BUILD >= CALC_FFMPEG_VERSION(52, 94, 100)
		AVDictionaryEntry* rotate_tag = av_dict_get(video_st->metadata, "rotate", NULL, 0);
		if (rotate_tag != NULL)
			rotation_angle = atoi(rotate_tag->value);
#endif
	}
}
