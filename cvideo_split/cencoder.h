﻿/***********************************************************************************************
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


#ifndef _C_ENCODER_H_
#define _C_ENCODER_H_
 
#include <stdint.h>
//#include <GL/eglew.h>
#include <vector>
#include <string>
#include <thread>
extern "C"
{
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace chen {

	class cencoder
	{
	public:
		explicit cencoder()
			: m_url("")
			, m_width(0)
			, m_height(0)
			, m_push_format_context_ptr(NULL)
			, m_codec_id(AV_CODEC_ID_NONE)
			, m_codec_ctx_ptr(NULL)
			, m_codec_ptr(NULL)
			, m_stream_ptr(NULL)
			, m_options_ptr(NULL)
			, m_pkt_ptr(NULL)
			, m_frame_ptr(NULL)
			, m_stoped(false)
		{}
		virtual ~cencoder(); 
	public:
		bool init( const char * url, uint32_t width, uint32_t height);

		void destroy();

		void stop();


	public:
		void consume_frame1(/*const AVFrame * frame_ptr*/
		 const uint8_t * data,int32_t step, int32_t width, uint32_t height, int32_t cn );
		void consume_frame2(/*const AVFrame * frame_ptr*/
			const uint8_t* data, int32_t step, int32_t width, uint32_t height, int32_t cn);
	private:
		void _work_pthread();
	private:
		std::string			m_url;
		uint32_t			m_width;
		uint32_t			m_height;
		AVFormatContext*	m_push_format_context_ptr;
		enum AVCodecID		m_codec_id;
		AVCodecContext*		m_codec_ctx_ptr;
		const AVCodec*			m_codec_ptr;
		AVStream*			m_stream_ptr;
		AVDictionary*		m_options_ptr; // 参数
		AVPacket*			m_pkt_ptr;
		AVFrame*			m_frame_ptr;
		bool				m_stoped;
		std::thread			m_thread;
		/*unsigned char* m_yuv420p_ptr;
		FILE* m_input_file_ptr;*/
	};
}

#endif // _C_ENCODER_H_