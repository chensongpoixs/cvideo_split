/***********************************************************************************************
					created: 		2019-05-13

					author:			chensong

					purpose:		msg_base_id
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


#ifndef _C_MPEGTS_ENCODER_H_
#define _C_MPEGTS_ENCODER_H_
#include "cnet_type.h"
#ifdef _MSC_VER
 
#include <WinSock2.h>
#elif defined(__GNUC__) 
 
#else
// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö§ï¿½ÖµÄ±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½Ô¼ï¿½Êµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
#error unexpected c complier (msc/gcc), Need to implement this method for demangle
return false;
#endif

#include "cmpegts_define.h"
#include <string>
namespace chen {

	class cmpegts_encoder
	{
	public:

		explicit cmpegts_encoder()
			: m_ip("")
			, m_port(0)
			, m_m2ts_mode(false)
			, m_pmt_start_pid(m_m2ts_mode == true ? (M2TS_PMT_PID): (0))
			, m_m2ts_video_pid(M2TS_VIDEO_PID)
			, m_m2ts_audio_pid(M2TS_AUDIO_START_PID)
			, m_m2ts_pgssub_pid(M2TS_PGSSUB_START_PID)
			, m_m2ts_textsub_pid(M2TS_TEXTSUB_PID)
			, m_max_delay(0)
			, m_pat()
			, m_sdt()
			, m_nit()
			, m_cc(15)
			, m_socket_fd(0)
			, m_servaddr()
			, m_send_buffer(NULL)
			, m_send_len(0)
		{}
		virtual ~cmpegts_encoder();

	public:
		bool init(const char * ip, uint32 port);
		void update(uint32 DataTime);
		void destroy();
	public:
		void push_packet(bool I_frame, uint8 * data, uint32 size, int64 pts, int64 dts);


	private:
		bool _send_packet(bool I_frame, uint8* payload, int32 payload_size, int64 pts, int64 dts);


		bool _mpegts_write_section(uint8* buff, int32 len);


		bool _send_ts_packet(uint8* payload, int32 payload_size);

		bool _mpegts_write_section1(int32 tid, int32 id, int32 version, int32 sec_num, int32 last_sec_num, uint8 * buff, int32 len);
	private:

		bool _mpegts_write_pat();
		/*
		* TODO@chensong 20240419   
		* Program Map Table（PMT）是 MPEG Transport Stream（MPEG-TS）协议中的一部分，用于描述特定节目的PID的内容，包含了特定PID对应的负载类型以及描述信息。以下是 PMT 的格式详细说明：

		1. PMT 表头部分：
			表标识符（Table Identifier）：一个字节，固定为0x02，用于标识该表为 PMT 表。
			语法段前向错误指示位（Section Syntax Indicator）：1位，指示表中是否包含语法段。
			零位（Zero）：1位，保留位，固定为0。
			保留位（Reserved）：2位，保留字段，固定为11。
			节目号（Program Number）：16位，标识该 PMT 表所描述的节目号。
			保留位（Reserved）：2位，保留字段，固定为11。
			版本号（Version Number）：5位，标识该 PMT 表的版本号。
			当前/下一个指示位（Current Next Indicator）：1位，指示该 PMT 表是当前有效还是下一个有效。
			节目所在段号（Section Number）：8位，指示当前 PMT 表所在的段号。
			总段数（Last Section Number）：8位，指示当前 PMT 表的总段数。
			CRC32 校验码（CRC32）：32位，用于校验 PMT 表的完整性。
		2. 节目信息段（Program Information Section）：
			节目描述长度（Program Info Length）：12位，描述节目信息段的长度。
			节目描述信息（Program Info Descriptor）：用于描述节目的附加信息，例如节目名称、提供商信息等。
		3. 流描述段（Stream Description Section）：
			流类型（Stream Type）：8位，描述数据流的类型，例如视频流、音频流等。
			保留位（Reserved）：3位，保留字段，固定为111。
			PID（Elementary PID）：13位，描述该数据流的PID。
			流描述长度（ES Info Length）：12位，描述流描述段的长度。
			流描述信息（ES Info Descriptor）：用于描述数据流的附加信息，例如编码格式、语言等。
		4. CRC32 校验码：
			32位的循环冗余检测码，用于校验整个 PMT 表的完整性。
		PMT 表中的节目信息段和流描述段可以根据需要重复出现多次，每个节目信息段描述一个节目，每个流描述段描述一个数据流。这样的设计使得 PMT 表能够描述一个节目中所有的数据流信息，为解析 MPEG-TS 数据流提供了重要的指导信息。
		
	//////////////////////////////////////////////////////////////////////////////////

		在 MPEG-TS（MPEG Transport Stream）协议中，PMT（Program Map Table）中的流描述段（Stream Description Section）包含了对特定 PID（Elementary PID）所对应数据流的描述信息。这些描述信息通常被称为 ES（Elementary Stream）描述符。以下是流描述段的格式详情：

		1. 流类型（Stream Type）：
			长度：8位。
			描述：用于指示数据流的类型，例如视频流、音频流、字幕流等。
			常见类型：
			0x02：视频流（MPEG-2 Video）
			0x03：音频流（MPEG-1 Audio）
			0x0F：AAC 音频流
			0x1B：H.264 视频流
			0x24：字幕流
		2. PID（Elementary PID）：
			长度：13位。
			描述：指示该数据流的 PID。
		3. 流描述长度（ES Info Length）：
			长度：12位。
			描述：指示流描述信息的长度，以字节为单位。
		4. 流描述信息（ES Info Descriptor）：
			描述：用于描述数据流的附加信息。
			格式：流描述信息包含了一个或多个描述符，每个描述符都有自己的格式和含义。
			常见描述符：
			语言描述符（Language Descriptor）：描述数据流的语言信息。
			最大码率描述符（Maximum Bitrate Descriptor）：描述数据流的最大传输码率。
			AC-3 描述符（AC-3 Descriptor）：针对 AC-3 音频流的描述信息。
			AAC 描述符（AAC Descriptor）：针对 AAC 音频流的描述信息。
			视频流标志描述符（Video Stream Descriptor）：描述视频流的附加信息，如宽高比、帧率等。
			音频流标志描述符（Audio Stream Descriptor）：描述音频流的附加信息，如采样率、声道数等。

		*/
		bool _mpegts_write_pmt();
		bool _mpegts_write_nit();
		bool _mpegts_write_sdt();

	private:
		std::string		m_ip;
		uint32			m_port;
		bool			m_m2ts_mode;
		int32			m_pmt_start_pid;
		int32			m_m2ts_video_pid;
		int32			m_m2ts_audio_pid;
		int32			m_m2ts_pgssub_pid;
		int32			m_m2ts_textsub_pid;
		int32			m_max_delay;
		// // round up to a whole number of TS packets  ===> 2930 
		// ts->pes_payload_size = (ts->pes_payload_size + 14 + 183) / 184 * 184 - 14;
		int32			m_pes_payload_size; 


		cmpeg_ts_section m_pat; // /* MPEG-2 PAT table */
		cmpeg_ts_section m_sdt; /* MPEG-2 SDT table context */
		cmpeg_ts_section m_nit; /* MPEG-2 NIT table context */
		int32			 m_cc;

		int32			 m_socket_fd;
		struct sockaddr_in m_servaddr;


		uint8* m_send_buffer;
		int32  m_send_len;

	};

}

#endif // _C_MPEGTS_ENCODER_H_