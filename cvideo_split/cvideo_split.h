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


#ifndef _C_VIDEO_SPLIT_H_
#define _C_VIDEO_SPLIT_H_

#include "cffmpeg_util.h"
#include "cnet_type.h"
#include "cnoncopytable.h"
#include <atomic>
#include "cencoder.h"
#include "cdecode.h"
#include <vector>
#include <map>
#include "cvideo_split_info_mgr.h"
#include <string>
namespace chen {

	struct cosd
	{
		uint32 x;
		uint32 y;
		uint32 fontsize;
		std::string fontcolor;
		std::string text;
		cosd (): x(0), y(0), fontsize(0), fontcolor(""), text(""){}
	};
	struct ccamera_info
	{
		uint32 index;
		double x;
		double y;
		double w;
		double h;
		/*std::string multicast_ip;
		uint32 multicast_port;*/
		std::string url; //媒体流
		ccamera_info() :index(0), x(0.0), y(0.0), w(0.0), h(0.0), url("") {}
	};

	class cvideo_splist : public cnoncopytable
	{
	public:
		explicit cvideo_splist()
			: m_stoped(false)
			, m_video_split_name("")
			, m_video_split_channel("")
			, m_multicast_ip("")
			, m_multicast_port(0)
			, m_split_method(0)
			, m_split_video_channels(0)
			, m_split_video_lock_1080p(false)
			, m_overlay(false)
			, m_osd()
			, m_out_video_width(1080)
			, m_out_video_height(1920)
			, m_camera_infos()
			, m_decodes()
			, m_encoder_ptr(NULL)
			, m_filter_graph_ptr(NULL)
			, m_buffers_ctx_ptr()
			, m_buffers_crop_ctx_ptr()
			, m_buffers_scale_ctx_ptr()
			, m_hstack_ctx_ptr(NULL)
			, m_vstack_ctx_ptr(NULL)
			, m_overlay_ctx_ptr(NULL)
			, m_osd_ctx_ptr(NULL)
			, m_buffersink_ctx_ptr(NULL)
			, m_decoder_frame_ptr(NULL)
			, m_encoder_frame_ptr(NULL)
			, m_filter_frame_ptr(NULL)
			, m_gpu_index(0)
			, m_id(-1)
		{}
		virtual ~cvideo_splist(){}

	private:
		typedef std::mutex							clock_type;
		typedef std::lock_guard<clock_type>			clock_guard;
	public:
		bool init(uint32 gpu_index, const VideoSplitInfo * video_split_info);
		void update(uint32 uDateTime);
		void destroy();


	public:
		uint32 get_use_gpu_index() const { return m_gpu_index; }
	private:

		bool _init_decodes(uint32 gpu_index);

		bool _init_filter();

		void _pthread_work();
		
		bool  _open();
	private:
		uint32					m_id;
		std::atomic_bool		m_stoped;
		// 视频拼接名称
		std::string				m_video_split_name;
		// 视频拼接通道  需要到第三方平台注册的通道id
		std::string				m_video_split_channel;
		// 组播地址
		std::string				m_multicast_ip;
		// 组播端口
		uint32					m_multicast_port;
		//拼接的方向 0: 左右 1:上下
		uint32					m_split_method; //
		//拼接视频的路数  [2 -10]
		uint32					m_split_video_channels;
		// 锁定1080P
		bool					m_split_video_lock_1080p;
		// 是否折叠
		bool					m_overlay;

		// OSD 
		cosd					m_osd;

		uint32					m_out_video_width;
		uint32					m_out_video_height;



		// 视频信息
		std::vector< ccamera_info> m_camera_infos;

		// 解码路数
		std::vector<cdecode*>       m_decodes;
	/*	cdecode* m_decode_ptr1;
		cdecode* m_decode_ptr2;*/
		
		//编码一路的编码器
		cencoder*					m_encoder_ptr;
		
		//视频拼接 的filter 芯片处理
		AVFilterGraph* m_filter_graph_ptr;
		
		//输入路数buffer
		std::vector<AVFilterContext*>	m_buffers_ctx_ptr;

		// crop 
		std::vector<AVFilterContext*>   m_buffers_crop_ctx_ptr;

		// scale
		std::vector<AVFilterContext*>   m_buffers_scale_ctx_ptr;

		// hstack 
		AVFilterContext* m_hstack_ctx_ptr;

		// vstack 
		AVFilterContext* m_vstack_ctx_ptr;

		// overlay 重合
		AVFilterContext* m_overlay_ctx_ptr;


		// OSD
		AVFilterContext* m_osd_ctx_ptr;

		// 输出 buffersink
		AVFilterContext* m_buffersink_ctx_ptr;



		// decoder Frame
		AVFrame* m_decoder_frame_ptr;
		// endoer Frame
		AVFrame* m_encoder_frame_ptr;
		

		std::thread			m_thread;
		AVFrame*			m_filter_frame_ptr;
		uint32				m_gpu_index;

		
	};


}

#endif // _C_VIDEO_SPLIT_H_