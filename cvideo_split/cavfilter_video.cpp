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


#include "cavfilter_video.h"
/*
namespace chen {

 





	CVideoMerge::CVideoMerge()
	{
		InitializeCriticalSection(&m_csVideoASection);
		InitializeCriticalSection(&m_csVideoBSection);
	}

	CVideoMerge::~CVideoMerge()
	{
		DeleteCriticalSection(&m_csVideoASection);
		DeleteCriticalSection(&m_csVideoBSection);
	}

	int CVideoMerge::StartMerge(const char *pFileA, const char *pFileB, const char *pFileOut)
	{
		avdevice_register_all();

		int ret = -1;
		do
		{
			ret = OpenFileA(pFileA);
			if (ret != 0)
			{
				break;
			}

			ret = OpenFileB(pFileB);
			if (ret != 0)
			{
				break;
			}

			ret = OpenOutPut(pFileOut);
			if (ret != 0)
			{
				break;
			}

			///这个滤镜的效果是以第一个视频做模板，且第一个视频的铺满全屏，第二个视频覆盖在第一个视频的右半部分
			//const char* filter_desc = "[in0]pad=1920:1080[x1];[in1]scale=w=960:h=1080[inn1];[x1][inn1]overlay=960:0[out]";


			const char* filter_desc = "[in0]split[main][tmp];[tmp]scale=w=960:h=1080[inn0];[main][inn0]overlay=0:0[x1];[in1]scale=w=960:h=1080[inn1];[x1][inn1]overlay=960:0[out]";

			ret = InitFilter(filter_desc);
			if (ret < 0)
			{
				break;
			}

			m_iYuv420FrameSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_pReadCodecCtx_VideoA->width, m_pReadCodecCtx_VideoA->height, 1);
			//申请30帧缓存
			m_pVideoAFifo = av_fifo_alloc(30 * m_iYuv420FrameSize);
			m_pVideoBFifo = av_fifo_alloc(30 * m_iYuv420FrameSize);

			m_hVideoAReadThread = CreateThread(NULL, 0, VideoAReadProc, this, 0, NULL);
			m_hVideoBReadThread = CreateThread(NULL, 0, VideoBReadProc, this, 0, NULL);

			m_hVideoMergeThread = CreateThread(NULL, 0, VideoMergeProc, this, 0, NULL);

		} while (0);

		return ret;
	}

	int CVideoMerge::WaitFinish()
	{
		int ret = 0;
		do
		{
			if (NULL == m_hVideoAReadThread || NULL == m_hVideoBReadThread)
			{
				break;
			}
			WaitForSingleObject(m_hVideoAReadThread, INFINITE);
			WaitForSingleObject(m_hVideoBReadThread, INFINITE);

			CloseHandle(m_hVideoAReadThread);
			m_hVideoAReadThread = NULL;
			CloseHandle(m_hVideoBReadThread);
			m_hVideoBReadThread = NULL;

			WaitForSingleObject(m_hVideoMergeThread, INFINITE);
			CloseHandle(m_hVideoMergeThread);
			m_hVideoMergeThread = NULL;
		} while (0);

		return ret;
	}

	int CVideoMerge::OpenFileA(const char *pFileA)
	{
		int ret = -1;

		do
		{
			if ((ret = avformat_open_input(&m_pFormatCtx_FileA, pFileA, 0, 0)) < 0) {
				printf("Could not open input file.");
				break;
			}
			if ((ret = avformat_find_stream_info(m_pFormatCtx_FileA, 0)) < 0) {
				printf("Failed to retrieve input stream information");
				break;
			}

			if (m_pFormatCtx_FileA->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
			{
				break;
			}
			m_pReadCodec_VideoA = (AVCodec *)avcodec_find_decoder(m_pFormatCtx_FileA->streams[0]->codecpar->codec_id);

			m_pReadCodecCtx_VideoA = avcodec_alloc_context3(m_pReadCodec_VideoA);

			if (m_pReadCodecCtx_VideoA == NULL)
			{
				break;
			}
			avcodec_parameters_to_context(m_pReadCodecCtx_VideoA, m_pFormatCtx_FileA->streams[0]->codecpar);

			m_pReadCodecCtx_VideoA->framerate = m_pFormatCtx_FileA->streams[0]->r_frame_rate;

			if (avcodec_open2(m_pReadCodecCtx_VideoA, m_pReadCodec_VideoA, NULL) < 0)
			{
				break;
			}

			ret = 0;
		} while (0);


		return ret;
	}

	int CVideoMerge::OpenFileB(const char *pFileB)
	{
		int ret = -1;

		do
		{
			if ((ret = avformat_open_input(&m_pFormatCtx_FileB, pFileB, 0, 0)) < 0) {
				printf("Could not open input file.");
				break;
			}
			if ((ret = avformat_find_stream_info(m_pFormatCtx_FileB, 0)) < 0) {
				printf("Failed to retrieve input stream information");
				break;
			}

			if (m_pFormatCtx_FileB->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
			{
				break;
			}
			m_pReadCodec_VideoB = (AVCodec *)avcodec_find_decoder(m_pFormatCtx_FileB->streams[0]->codecpar->codec_id);

			m_pReadCodecCtx_VideoB = avcodec_alloc_context3(m_pReadCodec_VideoB);

			if (m_pReadCodecCtx_VideoB == NULL)
			{
				break;
			}
			avcodec_parameters_to_context(m_pReadCodecCtx_VideoB, m_pFormatCtx_FileB->streams[0]->codecpar);

			m_pReadCodecCtx_VideoB->framerate = m_pFormatCtx_FileB->streams[0]->r_frame_rate;

			if (avcodec_open2(m_pReadCodecCtx_VideoB, m_pReadCodec_VideoB, NULL) < 0)
			{
				break;
			}

			ret = 0;
		} while (0);


		return ret;
	}


	int CVideoMerge::OpenOutPut(const char *pFileOut)
	{
		int iRet = -1;

		AVStream *pAudioStream = NULL;
		AVStream *pVideoStream = NULL;

		do
		{
			avformat_alloc_output_context2(&m_pFormatCtx_Out, NULL, NULL, pFileOut);

			{
				AVCodec* pCodecEncode_Video = (AVCodec *)avcodec_find_encoder(m_pFormatCtx_Out->oformat->video_codec);

				m_pCodecEncodeCtx_Video = avcodec_alloc_context3(pCodecEncode_Video);
				if (!m_pCodecEncodeCtx_Video)
				{
					break;
				}

				pVideoStream = avformat_new_stream(m_pFormatCtx_Out, pCodecEncode_Video);
				if (!pVideoStream)
				{
					break;
				}

				int frameRate = 10;
				m_pCodecEncodeCtx_Video->flags |= AV_CODEC_FLAG_QSCALE;
				m_pCodecEncodeCtx_Video->bit_rate = 4000000;
				m_pCodecEncodeCtx_Video->rc_min_rate = 4000000;
				m_pCodecEncodeCtx_Video->rc_max_rate = 4000000;
				m_pCodecEncodeCtx_Video->bit_rate_tolerance = 4000000;
				m_pCodecEncodeCtx_Video->time_base.den = frameRate;
				m_pCodecEncodeCtx_Video->time_base.num = 1;

				m_pCodecEncodeCtx_Video->width = m_iMergeWidth;
				m_pCodecEncodeCtx_Video->height = m_iMergeHeight;
				//pH264Encoder->pCodecCtx->frame_number = 1;
				m_pCodecEncodeCtx_Video->gop_size = 12;
				m_pCodecEncodeCtx_Video->max_b_frames = 0;
				m_pCodecEncodeCtx_Video->thread_count = 4;
				m_pCodecEncodeCtx_Video->pix_fmt = AV_PIX_FMT_YUV420P;
				m_pCodecEncodeCtx_Video->codec_id = AV_CODEC_ID_H264;
				m_pCodecEncodeCtx_Video->codec_type = AVMEDIA_TYPE_VIDEO;

				av_opt_set(m_pCodecEncodeCtx_Video->priv_data, "b-pyramid", "none", 0);
				av_opt_set(m_pCodecEncodeCtx_Video->priv_data, "preset", "superfast", 0);
				av_opt_set(m_pCodecEncodeCtx_Video->priv_data, "tune", "zerolatency", 0);

				if (m_pFormatCtx_Out->oformat->flags & AVFMT_GLOBALHEADER)
					m_pCodecEncodeCtx_Video->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

				if (avcodec_open2(m_pCodecEncodeCtx_Video, pCodecEncode_Video, 0) < 0)
				{
					//编码器打开失败，退出程序
					break;
				}
			}

			if (!(m_pFormatCtx_Out->oformat->flags & AVFMT_NOFILE))
			{
				if (avio_open(&m_pFormatCtx_Out->pb, pFileOut, AVIO_FLAG_WRITE) < 0)
				{
					break;
				}
			}

			avcodec_parameters_from_context(pVideoStream->codecpar, m_pCodecEncodeCtx_Video);

			if (avformat_write_header(m_pFormatCtx_Out, NULL) < 0)
			{
				break;
			}

			iRet = 0;
		} while (0);


		if (iRet != 0)
		{
			if (m_pCodecEncodeCtx_Video != NULL)
			{
				avcodec_free_context(&m_pCodecEncodeCtx_Video);
				m_pCodecEncodeCtx_Video = NULL;
			}

			if (m_pFormatCtx_Out != NULL)
			{
				avformat_free_context(m_pFormatCtx_Out);
				m_pFormatCtx_Out = NULL;
			}
		}

		return iRet;
	}

	//const char* filter_desc = "[in0]split[main][tmp];[tmp]scale=w=960:h=1080[inn0];[main][inn0]overlay=0:0[x1];[in1]scale=w=960:h=1080[inn1];[x1][inn1]overlay=960:0[out]";
	int CVideoMerge::InitFilter(const char* filter_desc)
	{
		int ret = 0;

		char args_pad[512];
		const char* pad_name_videoPad = "pad0";

		char args_videoA[512];
		const char* pad_name_videoA = "in0";
		char args_videoB[512];
		const char* pad_name_videoB = "in1";

		const char* scale_Tmp = "tmp";

		AVFilter* filter_src_videoPad = (AVFilter *)avfilter_get_by_name("pad");
		AVFilter* filter_src_videoA = (AVFilter *)avfilter_get_by_name("buffer");
		AVFilter* filter_src_videoB = (AVFilter *)avfilter_get_by_name("buffer");
		AVFilter* filter_sink = (AVFilter *)avfilter_get_by_name("buffersink");
		AVFilter *filter_split = (AVFilter *)avfilter_get_by_name("split");
		AVFilter *filter_overlayVideoA = (AVFilter *)avfilter_get_by_name("overlay");
		AVFilter *filter_overlayVideoB = (AVFilter *)avfilter_get_by_name("overlay");

		AVFilter *filter_scaleTmp = (AVFilter *)avfilter_get_by_name("scale");
		AVFilter *filter_scaleVideoB = (AVFilter *)avfilter_get_by_name("scale");


		AVFilterInOut* filter_output_videoPad = avfilter_inout_alloc();
		AVFilterInOut* filter_output_videoA = avfilter_inout_alloc();
		AVFilterInOut* filter_output_videoB = avfilter_inout_alloc();
		AVFilterInOut* filter_input = avfilter_inout_alloc();
		m_pFilterGraph = avfilter_graph_alloc();

		AVRational timeBase;
		timeBase.num = 1;
		timeBase.den = 10;


		AVRational timeAspect;
		timeAspect.num = 0;
		timeAspect.den = 1;


		_snprintf(args_pad, sizeof(args_pad), "width=%d:height=%d", 1920, 1080);


		_snprintf(args_videoA, sizeof(args_videoA),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			1920, 1080, AV_PIX_FMT_YUV420P,
			timeBase.num, timeBase.den,
			timeAspect.num,
			timeAspect.den);


		_snprintf(args_videoB, sizeof(args_videoB),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			1920, 1080, AV_PIX_FMT_YUV420P,
			timeBase.num, timeBase.den,
			timeAspect.num,
			timeAspect.den);


		AVFilterInOut* filter_outputs[3];
		do
		{
			AVFilterContext *splitFilter_ctx;
			ret = avfilter_graph_create_filter(&splitFilter_ctx, filter_split, "split", "outputs=2", NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			AVFilterContext *overlayVideoAFilter_ctx;
			ret = avfilter_graph_create_filter(&overlayVideoAFilter_ctx, filter_overlayVideoA, "overlayVideoA", "x=0:y=0", NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			AVFilterContext *overlayVideoBFilter_ctx;
			ret = avfilter_graph_create_filter(&overlayVideoBFilter_ctx, filter_overlayVideoB, "overlayVideoB", "x=960:y=0", NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			AVFilterContext *scaleTmpFilter_ctx;
			ret = avfilter_graph_create_filter(&scaleTmpFilter_ctx, filter_scaleTmp, "tmp", "w=960:h=1080", NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			AVFilterContext *scaleVideoBFilter_ctx;
			ret = avfilter_graph_create_filter(&scaleVideoBFilter_ctx, filter_scaleVideoB, "in1", "w=960:h=1080", NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}


			ret = avfilter_graph_create_filter(&m_pFilterCtxSrcVideoA, filter_src_videoA, pad_name_videoA, args_videoA, NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}
			ret = avfilter_graph_create_filter(&m_pFilterCtxSrcVideoB, filter_src_videoB, pad_name_videoB, args_videoB, NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			ret = avfilter_graph_create_filter(&m_pFilterCtxSink, filter_sink, "out", NULL, NULL, m_pFilterGraph);
			if (ret < 0)
			{
				break;
			}

			ret = av_opt_set_bin(m_pFilterCtxSink, "pix_fmts", (uint8_t*)&m_pCodecEncodeCtx_Video->pix_fmt, sizeof(m_pCodecEncodeCtx_Video->pix_fmt), AV_OPT_SEARCH_CHILDREN);



			//const char* filter_desc = "[in0]split[main][tmp];[tmp]scale=w=960:h=1080[inn0];[main][inn0]overlay=0:0[x1];[in1]scale=w=960:h=1080[inn1];[x1][inn1]overlay=960:0[out]";
			ret = avfilter_link(m_pFilterCtxSrcVideoA, 0, splitFilter_ctx, 0);
			if (ret != 0)
			{
				break;
			}

			ret = avfilter_link(splitFilter_ctx, 1, scaleTmpFilter_ctx, 0);
			if (ret != 0)
			{
				break;
			}

			ret = avfilter_link(splitFilter_ctx, 0, overlayVideoAFilter_ctx, 0);
			if (ret != 0)
			{
				break;
			}

			ret = avfilter_link(scaleTmpFilter_ctx, 0, overlayVideoAFilter_ctx, 1);
			if (ret != 0)
			{
				break;
			}


			ret = avfilter_link(m_pFilterCtxSrcVideoB, 0, scaleVideoBFilter_ctx, 0);
			if (ret != 0)
			{
				break;
			}


			ret = avfilter_link(overlayVideoAFilter_ctx, 0, overlayVideoBFilter_ctx, 0);
			if (ret != 0)
			{
				break;
			}
			ret = avfilter_link(scaleVideoBFilter_ctx, 0, overlayVideoBFilter_ctx, 1);
			if (ret != 0)
			{
				break;
			}

			ret = avfilter_link(overlayVideoBFilter_ctx, 0, m_pFilterCtxSink, 0);
			if (ret != 0)
			{
				break;
			}

			ret = avfilter_graph_config(m_pFilterGraph, NULL);
			if (ret < 0)
			{
				break;
			}

			ret = 0;

		} while (0);


		avfilter_inout_free(&filter_input);
		av_free(filter_src_videoA);
		av_free(filter_src_videoB);
		avfilter_inout_free(filter_outputs);

		char* temp = avfilter_graph_dump(m_pFilterGraph, NULL);

		return ret;
	}


	DWORD WINAPI CVideoMerge::VideoAReadProc(LPVOID lpParam)
	{
		CVideoMerge *pVideoMerge = (CVideoMerge *)lpParam;
		if (pVideoMerge != NULL)
		{
			pVideoMerge->VideoARead();
		}
		return 0;
	}

	void CVideoMerge::VideoARead()
	{
		AVFrame *pFrame;
		pFrame = av_frame_alloc();

		int y_size = m_pReadCodecCtx_VideoA->width * m_pReadCodecCtx_VideoA->height;

		AVPacket packet = { 0 };
		int ret = 0;
		while (1)
		{
			av_packet_unref(&packet);

			ret = av_read_frame(m_pFormatCtx_FileA, &packet);
			if (ret == AVERROR(EAGAIN))
			{
				continue;
			}
			else if (ret == AVERROR_EOF)
			{
				break;
			}
			else if (ret < 0)
			{
				break;
			}

			ret = avcodec_send_packet(m_pReadCodecCtx_VideoA, &packet);

			if (ret >= 0)
			{
				ret = avcodec_receive_frame(m_pReadCodecCtx_VideoA, pFrame);
				if (ret == AVERROR(EAGAIN))
				{
					continue;
				}
				else if (ret == AVERROR_EOF)
				{
					break;
				}
				else if (ret < 0) {
					break;
				}
				while (1)
				{
					if (av_fifo_space(m_pVideoAFifo) >= m_iYuv420FrameSize)
					{
						EnterCriticalSection(&m_csVideoASection);
						av_fifo_generic_write(m_pVideoAFifo, pFrame->data[0], y_size, NULL);
						av_fifo_generic_write(m_pVideoAFifo, pFrame->data[1], y_size / 4, NULL);
						av_fifo_generic_write(m_pVideoAFifo, pFrame->data[2], y_size / 4, NULL);
						LeaveCriticalSection(&m_csVideoASection);

						break;
					}
					else
					{
						Sleep(100);
					}
				}

			}


			if (ret == AVERROR(EAGAIN))
			{
				continue;
			}
		}

		av_frame_free(&pFrame);
	}

	DWORD WINAPI CVideoMerge::VideoBReadProc(LPVOID lpParam)
	{
		CVideoMerge *pVideoMerge = (CVideoMerge *)lpParam;
		if (pVideoMerge != NULL)
		{
			pVideoMerge->VideoBRead();
		}
		return 0;
	}

	void CVideoMerge::VideoBRead()
	{
		AVFrame *pFrame;
		pFrame = av_frame_alloc();

		int y_size = m_pReadCodecCtx_VideoB->width * m_pReadCodecCtx_VideoB->height;

		AVPacket packet = { 0 };
		int ret = 0;

		int iCount = 0;

		while (1)
		{
			av_packet_unref(&packet);

			ret = av_read_frame(m_pFormatCtx_FileB, &packet);
			if (ret == AVERROR(EAGAIN))
			{
				continue;
			}
			else if (ret == AVERROR_EOF)
			{
				break;
			}
			else if (ret < 0)
			{
				break;
			}

			ret = avcodec_send_packet(m_pReadCodecCtx_VideoB, &packet);

			if (ret >= 0)
			{
				ret = avcodec_receive_frame(m_pReadCodecCtx_VideoB, pFrame);
				if (ret == AVERROR(EAGAIN))
				{
					continue;
				}
				else if (ret == AVERROR_EOF)
				{
					break;
				}
				else if (ret < 0) {
					break;
				}

				while (1)
				{
					if (av_fifo_space(m_pVideoBFifo) >= m_iYuv420FrameSize)
					{
						EnterCriticalSection(&m_csVideoBSection);
						av_fifo_generic_write(m_pVideoBFifo, pFrame->data[0], y_size, NULL);
						av_fifo_generic_write(m_pVideoBFifo, pFrame->data[1], y_size / 4, NULL);
						av_fifo_generic_write(m_pVideoBFifo, pFrame->data[2], y_size / 4, NULL);
						LeaveCriticalSection(&m_csVideoBSection);

						break;
					}
					else
					{
						Sleep(100);
					}
				}
			}


			if (ret == AVERROR(EAGAIN))
			{
				continue;
			}
		}

		av_frame_free(&pFrame);
	}


	DWORD WINAPI CVideoMerge::VideoMergeProc(LPVOID lpParam)
	{
		CVideoMerge *pVideoMerge = (CVideoMerge *)lpParam;
		if (pVideoMerge != NULL)
		{
			pVideoMerge->VideoMerge();
		}
		return 0;
	}


	void CVideoMerge::VideoMerge()
	{
		int ret = 0;

		AVFrame *pFrameVideoA = av_frame_alloc();
		uint8_t *videoA_buffer_yuv420 = (uint8_t *)av_malloc(m_iYuv420FrameSize);
		av_image_fill_arrays(pFrameVideoA->data, pFrameVideoA->linesize, videoA_buffer_yuv420, AV_PIX_FMT_YUV420P, m_pReadCodecCtx_VideoA->width, m_pReadCodecCtx_VideoA->height, 1);


		AVFrame *pFrameVideoB = av_frame_alloc();
		uint8_t *videoB_buffer_yuv420 = (uint8_t *)av_malloc(m_iYuv420FrameSize);
		av_image_fill_arrays(pFrameVideoB->data, pFrameVideoB->linesize, videoB_buffer_yuv420, AV_PIX_FMT_YUV420P, m_pReadCodecCtx_VideoB->width, m_pReadCodecCtx_VideoB->height, 1);


		int iOutVideoWidth = m_pReadCodecCtx_VideoB->width;
		int iOutVideoHeight = m_pReadCodecCtx_VideoB->height;

		AVPacket packet = { 0 };
		int iPicCount = 0;

		AVFrame* pFrame_out = av_frame_alloc();
		uint8_t *out_buffer_yuv420 = (uint8_t *)av_malloc(m_iYuv420FrameSize);
		av_image_fill_arrays(pFrame_out->data, pFrame_out->linesize, out_buffer_yuv420, AV_PIX_FMT_YUV420P, m_pReadCodecCtx_VideoA->width, m_pReadCodecCtx_VideoA->height, 1);

		while (1)
		{
			if (NULL == m_pVideoAFifo)
			{
				break;
			}
			if (NULL == m_pVideoBFifo)
			{
				break;
			}

			int iVideoASize = av_fifo_size(m_pVideoAFifo);
			int iVideoBSize = av_fifo_size(m_pVideoBFifo);

			if (iVideoASize >= m_iYuv420FrameSize && iVideoBSize >= m_iYuv420FrameSize)
			{
				EnterCriticalSection(&m_csVideoASection);
				av_fifo_generic_read(m_pVideoAFifo, videoA_buffer_yuv420, m_iYuv420FrameSize, NULL);
				LeaveCriticalSection(&m_csVideoASection);


				EnterCriticalSection(&m_csVideoBSection);
				av_fifo_generic_read(m_pVideoBFifo, videoB_buffer_yuv420, m_iYuv420FrameSize, NULL);
				LeaveCriticalSection(&m_csVideoBSection);


				pFrameVideoA->pkt_dts = pFrameVideoA->pts = av_rescale_q_rnd(iPicCount, m_pCodecEncodeCtx_Video->time_base, m_pFormatCtx_Out->streams[0]->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				pFrameVideoA->pkt_duration = 0;
				pFrameVideoA->pkt_pos = -1;

				pFrameVideoA->width = iOutVideoWidth;
				pFrameVideoA->height = iOutVideoHeight;
				pFrameVideoA->format = AV_PIX_FMT_YUV420P;



				pFrameVideoB->pkt_dts = pFrameVideoB->pts = av_rescale_q_rnd(iPicCount, m_pCodecEncodeCtx_Video->time_base, m_pFormatCtx_Out->streams[0]->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				pFrameVideoB->pkt_duration = 0;
				pFrameVideoB->pkt_pos = -1;

				pFrameVideoB->width = iOutVideoWidth;
				pFrameVideoB->height = iOutVideoHeight;
				pFrameVideoB->format = AV_PIX_FMT_YUV420P;

				//if (iPicCount == 0)
				{
					printf("begin av_buffersrc_add_frame");
					//ret = av_buffersrc_add_frame_flags(m_pFilterCtxSrcVideoA, pFrameVideoA, 0);
					ret = av_buffersrc_add_frame(m_pFilterCtxSrcVideoA , pFrameVideoA);
					if (ret < 0)
					{
						break;
					}

					//ret = av_buffersrc_add_frame_flags(m_pFilterCtxSrcVideoB, pFrameVideoB, 0);
					ret = av_buffersrc_add_frame(m_pFilterCtxSrcVideoB, pFrameVideoB);
					if (ret < 0)
					{
						break;
					}
				}

				do
				{
					//while (1)
					{
						ret = av_buffersink_get_frame(m_pFilterCtxSink, pFrame_out);
						if (ret < 0)
						{
							//printf("Mixer: failed to call av_buffersink_get_frame_flags\n");
							break;
						}

						printf("end av_buffersink_get_frame_flags");

						pFrame_out->pkt_dts = pFrame_out->pts = av_rescale_q_rnd(iPicCount, m_pCodecEncodeCtx_Video->time_base, m_pFormatCtx_Out->streams[0]->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						pFrame_out->pkt_duration = 0;
						pFrame_out->pkt_pos = -1;

						pFrame_out->width = iOutVideoWidth;
						pFrame_out->height = iOutVideoHeight;
						pFrame_out->format = AV_PIX_FMT_YUV420P;

						ret = avcodec_send_frame(m_pCodecEncodeCtx_Video, pFrame_out);

						ret = avcodec_receive_packet(m_pCodecEncodeCtx_Video, &packet);

						av_write_frame(m_pFormatCtx_Out, &packet);

						iPicCount++;

						av_frame_unref(pFrame_out);
					}

				} while (0);
			}
			else
			{
				if (m_hVideoAReadThread == NULL && m_hVideoBReadThread == NULL)
				{
					break;
				}
				Sleep(1);
			}
		}

		av_write_trailer(m_pFormatCtx_Out);
		avio_close(m_pFormatCtx_Out->pb);

		av_frame_free(&pFrame_out);

		av_frame_free(&pFrameVideoA);
		av_frame_free(&pFrameVideoB);
	}







}*/
