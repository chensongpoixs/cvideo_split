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


#ifndef _C_DECODE_H_
#define _C_DECODE_H_
 
#include "cffmpeg_util.h"
#include "cnet_type.h"
#include <list>
#include <thread>
namespace chen {


	class cdecode
	{
	public:
		explicit cdecode()
			: m_open(false)
			, m_url("")
			, m_width(0)
			, m_height(0)
			, m_gpu_index(0)
			, m_dict(NULL)
			, m_video_stream_index(-1)
			, m_video_stream_ptr(NULL)
			, m_pixfmt(AV_PIX_FMT_NONE)
			, m_ic_ptr(NULL)
			, m_codec_ctx_ptr(NULL)
			, m_packet_ptr(NULL)
			, m_picture_ptr(NULL)
			, m_picture_pts(AV_NOPTS_VALUE)
			, m_frame()
			, m_sws_frame_ptr(NULL)
			, m_sws_ctx_ptr(NULL)
			, m_frame_number(-1)
			, m_first_frame_number(-1)
			, m_rotation_auto(false)
			, m_rotation_angle(-1)
			, m_eps_zero(0.000025)
			, m_open_timeout(LIBAVFORMAT_INTERRUPT_OPEN_DEFAULT_TIMEOUT_MS)
			, m_read_timeout(LIBAVFORMAT_INTERRUPT_READ_DEFAULT_TIMEOUT_MS)
			, m_interrupt_metadata()
			, m_dts(0)
			, m_pts(0)
			, m_index(0)
			, m_stoped(false)
			, m_frame_list()
			{}
		virtual ~cdecode();
	public:
		/**
		 * @param url 文件路径
		* @param fmt 输出像素格式
		* @return 成功返回true 失败返回false
		*/
		bool init(uint32 gpu_index, const char * url, uint32 index);
		void destroy();



		// 关闭并释放资源
		void close();

		/**
		* 读取一帧视频数据
		* @param out_frame 输出帧数据，原始像素格式
		* @return -1 错误, -2 没有打开、0：获取到结尾，1:获取成功，
		*/
		bool grab_frame( );

		/**
		* 读取一帧视频数据并转换到目标像素格式
		* @param out_frame 输出帧数据， 目标像素格式， open函数设置
		*/
		bool retrieve(AVFrame * &frame
					   /*unsigned char** data, int* step, int* width,
			int* height, int* cn */);


		bool get_frame(AVFrame*& frame);

		/**
		* seek到目标位置相近的关键帧
		* @param percentage 目标位置 ， 百分比[0 ~1)
		* @return 成功返回 true
		*/
		bool seek(double sec);

		void seek(int64_t frame_number);

		
	public:
		int get_width() const { return m_width; }
		int get_height() const { return m_height; }


	public:
		 
		int64_t get_total_frames() const;
		double  get_duration_sec() const;
		double  get_fps() const;
		int64_t get_bitrate() const;

		double  r2d(AVRational r) const;
		int64_t dts_to_frame_number(int64_t dts);
		double  dts_to_sec(int64_t dts) const;
		void    get_rotation_angle();
		uint64  get_dts()const { return m_dts; };
		uint64  get_pts() const {
			return m_pts;;
		}
	private:
		//cdecode(const cdecode&);



	private:
		void _pthread_decoder();
	public:
		bool   m_open;
		std::string m_url;
		int	   m_width;
		int	   m_height;
		uint32				m_gpu_index;
		AVDictionary*		m_dict;
		// 视频流索引
		int32_t	   m_video_stream_index;
		// 视频流
		AVStream* m_video_stream_ptr;

		AVPixelFormat	m_pixfmt;

		//接封装上下文
		AVFormatContext* m_ic_ptr;
		// 解码器上下文
		AVCodecContext*	m_codec_ctx_ptr;
		AVPacket*		m_packet_ptr;
		AVFrame*		m_picture_ptr;

		int64_t			m_picture_pts;

		cimage_ffmpeg	m_frame;
		AVFrame*		m_sws_frame_ptr;
		
		//像素格式转换上下文
		SwsContext*		m_sws_ctx_ptr;

		int64_t			m_frame_number;
		int64_t			m_first_frame_number;
		bool			m_rotation_auto;
		int32_t			m_rotation_angle; // valid 0, 90, 180, 270
		double			m_eps_zero;


		int32_t			m_open_timeout;
		int32_t			m_read_timeout;
		AVInterruptCallbackMetadata m_interrupt_metadata;
		uint64			m_dts;
		uint32			m_pts;
		uint32			m_index;
		std::thread		m_thread;
		bool			m_stoped;
		 std::mutex  m_frame_lock;
		std::list<AVFrame*> m_frame_list;
	};

}

#endif // _C_DECODE_H_
