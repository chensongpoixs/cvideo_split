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
		//frames_ctx->sw_format = AV_PIX_FMT_NV12;
		frames_ctx->sw_format = AV_PIX_FMT_YUV420P;
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
	 bool cencoder::init(  const char* url,  uint32_t width, uint32_t height)
	{
		int ret = 0;
	//	if (m_stoped)
		{
			destroy();
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
		
		m_push_format_context_ptr = avformat_alloc_context();
		if (!m_push_format_context_ptr)
		{
			printf("alloc  avformat context  failed !!! \n");
			return false;
		}
		ret = avformat_alloc_output_context2(&m_push_format_context_ptr,  NULL, "mpegts", std::string(m_url + "?pkt_size=1316").c_str());
		if (ret < 0)
		{
			printf("avformat alloc output context2  failed !!! [ret = %s]\n", ffmpeg_util::make_error_string(ret));
			return false;
		}

		m_stream_ptr = avformat_new_stream(m_push_format_context_ptr, NULL); //分配流空间
		if (!m_stream_ptr)
		{
			printf("[%s] alloc stream failed !!!\n", m_url.c_str());
			return false;
		}
		//::avcodec_register_all();
		if (m_push_format_context_ptr->oformat->flags & AVFMT_GLOBALHEADER)
		{
			m_push_format_context_ptr->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		m_codec_id = AV_CODEC_ID_H264;
		AVBufferRef* hw_device_ctx = NULL;
		 ret  = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA,
			NULL, NULL, 0);
		//m_codec_ptr = ::avcodec_find_encoder(m_codec_id);
		m_codec_ptr = ::avcodec_find_encoder_by_name("h264_nvenc");
		if (!m_codec_ptr)
		{
			printf("Could not find [encoder = %s]   failed !!!\n", ::avcodec_get_name(m_codec_id));
			return false;
		}
		m_codec_ctx_ptr = avcodec_alloc_context3(m_codec_ptr);
		if (!m_codec_ctx_ptr)
		{
			printf("Conlu not alloc video context (%s)failed !!!\n", m_url.c_str());
			return false;
		}
		//avctx->width = width;
		//avctx->height = height;
		//avctx->time_base = (AVRational){ 1, 25 };
		//avctx->framerate = (AVRational){ 25, 1 };
		//avctx->sample_aspect_ratio = (AVRational){ 1, 1 };
		//avctx->pix_fmt = AV_PIX_FMT_VAAPI;

		m_codec_ctx_ptr->bit_rate = 4000 * 1024;//m_width * m_height * 25 *1;
		m_codec_ctx_ptr->width = m_width;
		m_codec_ctx_ptr->height = m_height;
		AVRational rate;
		rate.num = 1;
		rate.den = 25;
		m_codec_ctx_ptr->time_base = rate;
		m_codec_ctx_ptr->gop_size = 300;
		m_codec_ctx_ptr->max_b_frames = 0;
		m_codec_ctx_ptr->pix_fmt = AV_PIX_FMT_CUDA;
		{
			/* set hw_frames_ctx for encoder's AVCodecContext */
			if ((ret = set_hwframe_ctx(m_codec_ctx_ptr, hw_device_ctx)) < 0) {
				fprintf(stderr, "Failed to set hwframe context.\n");
				//goto close;
			}
		
		}
		//m_codec_ctx_ptr->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		m_pts = 0;
		m_frame_count = 0;
		m_mic = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now().time_since_epoch());
		av_opt_set(m_codec_ctx_ptr->priv_data, "preset", "slow", 0);
		//设置零延迟(本地摄像头视频流保存如果不设置则播放的时候会越来越模糊)
		//av_opt_set(m_codec_ctx_ptr->priv_data, "tune", "zerolatency", 0);
		//av_dict_set(m_codec_ctx_ptr->priv_data, "preset", "slow", 0);
		//open the encoder
		//AVDictionary* options = NULL;
		//if (!av_dict_get(options, "threads", NULL, 0))
		//	av_dict_set(&options, "threads", "auto", 0);

		//if (m_codec_ptr->capabilities & AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE) {
		//	ret = av_dict_set(&options, "flags", "+copy_opaque", AV_DICT_MULTIKEY);
		//	if (ret < 0)
		//		return ret;
		//}

		//av_dict_set(&options, "flags", "+frame_duration", AV_DICT_MULTIKEY);

		//const AVCodecHWConfig* config;
		////HWDevice* dev = NULL;
		//int i;
		//AVBufferRef* device_ref = NULL;
		//for (i = 0;; i++) {
		//	config = avcodec_get_hw_config(m_codec_ptr, i);
		//	if (!config)
		//	{
		//		break;
		//	}

		//	/*if (frames_ref &&
		//		config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX &&
		//		(config->pix_fmt == AV_PIX_FMT_NONE ||
		//			config->pix_fmt == ost->enc_ctx->pix_fmt)) {
		//		av_log(ost->enc_ctx, AV_LOG_VERBOSE, "Using input "
		//			"frames context (format %s) with %s encoder.\n",
		//			av_get_pix_fmt_name(ost->enc_ctx->pix_fmt),
		//			ost->enc_ctx->codec->name);
		//		ost->enc_ctx->hw_frames_ctx = av_buffer_ref(frames_ref);
		//		if (!ost->enc_ctx->hw_frames_ctx)
		//			return AVERROR(ENOMEM);
		//		return 0;
		//	}*/

		//	/*if (!dev &&
		//		config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
		//	{
		//		dev = hw_device_get_by_type(config->device_type);
		//	}*/
		//	//if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
		//	//{
		//	//	int32_t err = av_hwdevice_ctx_create(&device_ref, AV_HWDEVICE_TYPE_CUDA,
		//	//		NULL, NULL/*options*/, 0);
		//	//	//dev = hw_device_get_by_type(config->device_type);
		//	//	m_codec_ctx_ptr->hw_device_ctx = av_buffer_ref(device_ref);
		//	//	break;
		//	//}
		//	
		//}

		ret = avcodec_open2(m_codec_ctx_ptr, m_codec_ptr, NULL);
		if (ret < 0 ) 
		{
			printf( "[%s]Open encoder (%s)failed !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
			return false;
		}

		//av_lockmgr_register();
		ret = avcodec_parameters_from_context(m_push_format_context_ptr->streams[0]->codecpar, m_codec_ctx_ptr);
		if (ret < 0) 
		{
			printf(  "[%s]Failed to copy encoder parameters to output stream #%s\n", m_url.c_str(),  ffmpeg_util::make_error_string(ret));
			return false;
		}
		// 设置 mpeg ts page size 包一定要是 1316  在vlc中才能解析
		ret = av_dict_set(&m_options_ptr, "pkt_size", "1316", AVIO_FLAG_WRITE);
		const AVDictionaryEntry* e = NULL; 
		while (e = av_dict_get(m_options_ptr, "", e, AV_DICT_IGNORE_SUFFIX))
		{
			printf("[key = %s][value = %s]\n", e->key, e->value);
		}


		//****分配推流io上下文****//
		ret = ::avio_open2(&m_push_format_context_ptr->pb, m_url.c_str(), AVIO_FLAG_WRITE, NULL, &m_options_ptr);
		if (ret < 0) 
		{
			printf("Could not open output file '%s' [%s]\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
			return false;
		}
		//写入头
		ret = avformat_write_header(m_push_format_context_ptr, NULL); //写入头
		if (ret < 0) 
		{
			printf("[%s]Error occurred when opening output file [%s]\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
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
			printf("[url = %s]  alloc pakcet failed !!!\n", m_url.c_str());
			return false;
		}
	/*	m_yuv420p_ptr = static_cast<unsigned char *>(::malloc(sizeof(unsigned char) * (m_width * m_height * 471 * 2)));
		
		uint32_t frame_size = m_width * m_height * 3 / 2;
		::fread(m_yuv420p_ptr, 1, frame_size * 471, m_input_file_ptr);*/

		//m_thread = std::thread(&cencoder::_work_pthread, this);
		return true;
	}

	void cencoder::destroy()
	{
		if (!m_stoped)
		{
			printf("[url = %s] stop = false\n", m_url.c_str());
		//	return;
		}
		

	}

	void cencoder::stop()
	{
		m_stoped = true;
	}

	void cencoder::push_frame(  AVFrame* frame_ptr)
	{
		if (m_stoped)
		{
			//::av_frame_unref(frame_ptr);
			return;
		}
		int32_t ret = 0;

		m_pkt_ptr->data = NULL;
		m_pkt_ptr->size = 0;
		m_pts += (100000 / 120);
		std::chrono::microseconds current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()) - m_mic;
		//frame_ptr->pts = (current_mic.count() / 10) + AV_TIME_BASE;
		++m_frame_count;
		if (m_frame_count == 1000)
		{
			m_frame_count = 0;
		}
		//if (m_codec_ctx_ptr->hw_device_ctx)
		//{
		//	// copy data to HW frame
		//	AVFrame* hw_frame = av_frame_alloc();
		//	if (!hw_frame) {
		//		//CV_LOG_ERROR(NULL, "Error allocating AVFrame (av_frame_alloc)");
		//		return  ;
		//	}
		//	if (av_hwframe_get_buffer(m_codec_ctx_ptr->hw_frames_ctx, hw_frame, 0) < 0) {
		//		//CV_LOG_ERROR(NULL, "Error obtaining HW frame (av_hwframe_get_buffer)");
		//		av_frame_free(&hw_frame);
		//		return  ;
		//	}
		//	if (av_hwframe_transfer_data(hw_frame, frame_ptr, 0) < 0) {
		//		//CV_LOG_ERROR(NULL, "Error copying data from CPU to GPU (av_hwframe_transfer_data)");
		//		av_frame_free(&hw_frame);
		//		return  ;
		//	}
		//	hw_frame->pts = 1;
		//	ret = ::avcodec_send_frame(m_codec_ctx_ptr, hw_frame);
		//	if (ret < 0)
		//	{
		//		//::av_frame_unref(frame_ptr);
		//		printf("[warr][url = %s] codec send frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
		//		return;
		//	}
		//	av_frame_free(&hw_frame);
		//}
		//else
		{
			AVFrame* hw_frame = NULL;
			if (!(hw_frame = av_frame_alloc())) {
				ret = AVERROR(ENOMEM);
				return;
			}
			if ((ret = av_hwframe_get_buffer(m_codec_ctx_ptr->hw_frames_ctx, hw_frame, 0)) < 0) {
				fprintf(stderr, "Error code: %s.\n", ffmpeg_util::make_error_string(ret));
				return;
			}
			if (!hw_frame->hw_frames_ctx) {
				ret = AVERROR(ENOMEM);
				return;
			}

			if ((ret = av_hwframe_transfer_data(hw_frame, frame_ptr, 0)) < 0) {
				fprintf(stderr, "Error while transferring frame data to surface."
					"Error code: %s.\n", ffmpeg_util::make_error_string(ret));
				return;
			}

			ret = ::avcodec_send_frame(m_codec_ctx_ptr, hw_frame);
			if (ret < 0)
			{
				//::av_frame_unref(frame_ptr);
				printf("[warr][url = %s] codec send frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
				return;
			}
			av_frame_free(&hw_frame);
		}

		
		//::av_frame_unref(frame_ptr);
		//::av_frame_free(&frame_ptr);
		//frame_ptr = NULL;
		ret = ::avcodec_receive_packet(m_codec_ctx_ptr, m_pkt_ptr);
		if (ret < 0)
		{
			av_packet_unref(m_pkt_ptr);
			printf("[warr][url = %s]codec  receive packet  (%s) failed !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
			return;

		}

		current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()) - m_mic;
		m_pkt_ptr->pts = frame_ptr->pts;// (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.pts;// + (int)(duration*AV_TIME_BASE);
		
		m_pkt_ptr->dts = frame_ptr->pkt_dts; // (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.dts;// + (int)(duration*AV_TIME_BASE);

		m_pkt_ptr->stream_index = 0;
		printf(" [frame = %lu][%lu]  [pts = %lu][%lu ms]\n", m_frame_count, ::time(NULL), m_pkt_ptr->dts, current_mic.count());

		//1706158630000000 >= 1706158630000000

		//printf("pts:%d , dts:%d , duration*AV_TIME_BASE:%d\n", encodePacket.pts, encodePacket.dts, (int)(duration * AV_TIME_BASE));

		//av_packet_rescale_ts(m_pkt_ptr, pInStream->time_base, pOutStream->time_base);

		//m_pkt_ptr->pos = -1;
		//printf("[frame = %u][dts = %u]\n", frame_count, m_pkt_ptr->dts);
		// 4. 将编码后的packet写入输出媒体文件
		/*static FILE* out_file_ptr = ::fopen("chensong.h264", "wb+");
		if (out_file_ptr)
		{
			::fwrite(m_pkt_ptr->data, 1, m_pkt_ptr->size, out_file_ptr);
			fflush(out_file_ptr);
		}*/
		ret = av_interleaved_write_frame(m_push_format_context_ptr, m_pkt_ptr);
		if (ret < 0)
		{
			printf("[error][url = %s] interleaved write frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
		}
		m_mic = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		//AVFrame* temp = NULL;
		//::av_frame_move_ref(temp, frame_ptr);
		//if (!temp)
		//{// warr
		//	return;
		//}
		//clock_guard lock(m_frame_lock);
		//m_frame_list.push_back(temp);
		/*memcpy(m_frame_ptr->data[0], frame_ptr->data[0], frame_ptr->width * frame_ptr->height);
		memcpy(m_frame_ptr->data[1], frame_ptr->data[1], frame_ptr->width * frame_ptr->height/4);
		memcpy(m_frame_ptr->data[2], frame_ptr->data[2], frame_ptr->width * frame_ptr->height/4);*/

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
					printf("[warr][url = %s] codec send frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
					continue;
				}
				::av_frame_free(&frame_ptr);
				frame_ptr = NULL;
				ret = ::avcodec_receive_packet(m_codec_ctx_ptr, m_pkt_ptr);
				if (ret < 0)
				{
					printf("[warr][url = %s]codec  receive packet  (%s) failed !!!\n", m_url.c_str(), ffmpeg_util::make_error_string(ret));
					continue;

				}

				current_mic = std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::system_clock::now().time_since_epoch()) - mic;
				m_pkt_ptr->pts = (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.pts;// + (int)(duration*AV_TIME_BASE);

				m_pkt_ptr->dts = (current_mic.count() / 10) + AV_TIME_BASE; // decodePacket.dts;// + (int)(duration*AV_TIME_BASE);

				m_pkt_ptr->stream_index = 0;
				printf(" [frame = %lu][%lu]  [pts = %lu]\n", frame_count, ::time(NULL), m_pkt_ptr->dts);

				//1706158630000000 >= 1706158630000000

				//printf("pts:%d , dts:%d , duration*AV_TIME_BASE:%d\n", encodePacket.pts, encodePacket.dts, (int)(duration * AV_TIME_BASE));

				//av_packet_rescale_ts(m_pkt_ptr, pInStream->time_base, pOutStream->time_base);

				//m_pkt_ptr->pos = -1;
				//printf("[frame = %u][dts = %u]\n", frame_count, m_pkt_ptr->dts);
				// 4. 将编码后的packet写入输出媒体文件
				ret = av_interleaved_write_frame(m_push_format_context_ptr, m_pkt_ptr);
				if (ret < 0)
				{
					printf("[error][url = %s] interleaved write frame (%s) failed !!!", m_url.c_str(), ffmpeg_util::make_error_string(ret));
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

}
