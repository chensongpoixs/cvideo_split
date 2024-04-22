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


#ifndef _C_MPEGTS_DEFINE_H_
#define _C_MPEGTS_DEFINE_H_
#include "cnet_type.h"
namespace chen {

	/* m2ts pids */
//#define M2TS_PMT_PID                      0x0100
//#define M2TS_PCR_PID                      0x1001
//#define M2TS_VIDEO_PID                    0x1011
//#define M2TS_AUDIO_START_PID              0x1100
//#define M2TS_PGSSUB_START_PID             0x1200
//#define M2TS_TEXTSUB_PID                  0x1800
//#define M2TS_SECONDARY_AUDIO_START_PID    0x1A00
//#define M2TS_SECONDARY_VIDEO_START_PID    0x1B00

	/* mpegts writer */

	static const char* DEFAULT_PROVIDER_NAME = "chen_mpegts";
	static const char* DEFAULT_SERVICE_NAME  =  "Service";

	static const uint32  M2TS_PMT_PID = 0x0100;


	static const uint32   TS_FEC_PACKET_SIZE = 204;
	static const uint32   TS_DVHS_PACKET_SIZE = 192;
	static const uint32   TS_PACKET_SIZE = 188;
	static const uint32   TS_MAX_PACKET_SIZE = 204;


	/* The section length is 12 bits. The first 2 are set to 0, the remaining
 * 10 bits should not exceed 1021. */
	static const uint32   SECTION_LENGTH = 1020;

	static const uint32 M2TS_PCR_PID                   =   0x1001 ;
	static const uint32 M2TS_VIDEO_PID                 =   0x1011 ;
	static const uint32 M2TS_AUDIO_START_PID           =   0x1100 ;
	static const uint32 M2TS_PGSSUB_START_PID          =   0x1200 ;
	static const uint32 M2TS_TEXTSUB_PID               =   0x1800 ;
	static const uint32 M2TS_SECONDARY_AUDIO_START_PID =   0x1A00 ;
	static const uint32 M2TS_SECONDARY_VIDEO_START_PID =   0x1B00 ;


	/* pids */
	static const uint32  PAT_PID = 0x0000; /* Program Association Table */
	static const uint32  CAT_PID = 0x0001; /* Conditional Access Table */
	static const uint32  TSDT_PID = 0x0002; /* Transport Stream Description Table */

	/* PID from 0x0004 to 0x000F are reserved */
	static const uint32  NIT_PID         = 0x0010; /* Network Information Table */
	static const uint32  SDT_PID         = 0x0011; /* Service Description Table */
	static const uint32  BAT_PID         = 0x0011; /* Bouquet Association Table */
	static const uint32  EIT_PID         = 0x0012; /* Event Information Table */
	static const uint32  RST_PID         = 0x0013; /* Running Status Table */
	static const uint32  TDT_PID         = 0x0014; /* Time and Date Table */
	static const uint32  TOT_PID         = 0x0014;
	static const uint32  NET_SYNC_PID    = 0x0015;
	static const uint32  RNT_PID         = 0x0016; /* RAR Notification Table */
	/* PID from 0x0017 to 0x001B are reserved for future use */


	/* table ids */
#define PAT_TID         0x00 /* Program Association section */
#define CAT_TID         0x01 /* Conditional Access section */
#define PMT_TID         0x02 /* Program Map section */
#define TSDT_TID        0x03 /* Transport Stream Description section */
/* TID from 0x04 to 0x3F are reserved */
#define M4OD_TID        0x05
#define NIT_TID         0x40 /* Network Information section - actual network */
#define ONIT_TID        0x41 /* Network Information section - other network */
#define SDT_TID         0x42 /* Service Description section - actual TS */
/* TID from 0x43 to 0x45 are reserved for future use */
#define OSDT_TID        0x46 /* Service Descrition section - other TS */
/* TID from 0x47 to 0x49 are reserved for future use */
#define BAT_TID         0x4A /* Bouquet Association section */
#define UNT_TID         0x4B /* Update Notification Table section */
#define DFI_TID         0x4C /* Downloadable Font Info section */
/* TID 0x4D is reserved for future use */
#define EIT_TID         0x4E /* Event Information section - actual TS */
#define OEIT_TID        0x4F /* Event Information section - other TS */
#define EITS_START_TID  0x50 /* Event Information section schedule - actual TS */
#define EITS_END_TID    0x5F /* Event Information section schedule - actual TS */
#define OEITS_START_TID 0x60 /* Event Information section schedule - other TS */
#define OEITS_END_TID   0x6F /* Event Information section schedule - other TS */
#define TDT_TID         0x70 /* Time Date section */
#define RST_TID         0x71 /* Running Status section */
#define ST_TID          0x72 /* Stuffing section */
#define TOT_TID         0x73 /* Time Offset section */
#define AIT_TID         0x74 /* Application Inforamtion section */
#define CT_TID          0x75 /* Container section */
#define RCT_TID         0x76 /* Related Content section */
#define CIT_TID         0x77 /* Content Identifier section */
#define MPE_FEC_TID     0x78 /* MPE-FEC section */
#define RPNT_TID        0x79 /* Resolution Provider Notification section */
#define MPE_IFEC_TID    0x7A /* MPE-IFEC section */
#define PROTMT_TID      0x7B /* Protection Message section */
/* TID from 0x7C to 0x7D are reserved for future use */
#define DIT_TID         0x7E /* Discontinuity Information section */
#define SIT_TID         0x7F /* Selection Information section */
/* TID from 0x80 to 0xFE are user defined */
/* TID 0xFF is reserved */

	/* ISO/IEC 13818-1 Table 2-22 */
#define STREAM_ID_PROGRAM_STREAM_MAP        0xbc
#define STREAM_ID_PRIVATE_STREAM_1          0xbd
#define STREAM_ID_PADDING_STREAM            0xbe
#define STREAM_ID_PRIVATE_STREAM_2          0xbf
#define STREAM_ID_AUDIO_STREAM_0            0xc0
#define STREAM_ID_VIDEO_STREAM_0            0xe0
#define STREAM_ID_ECM_STREAM                0xf0
#define STREAM_ID_EMM_STREAM                0xf1
#define STREAM_ID_DSMCC_STREAM              0xf2
#define STREAM_ID_TYPE_E_STREAM             0xf8
#define STREAM_ID_METADATA_STREAM           0xfc
#define STREAM_ID_EXTENDED_STREAM_ID        0xfd
#define STREAM_ID_PROGRAM_STREAM_DIRECTORY  0xff



	/* TID from 0x80 to 0xFE are user defined */
/* TID 0xFF is reserved */

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_METADATA        0x15
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_AVS2      0xd2
#define STREAM_TYPE_VIDEO_AVS3      0xd4
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83
#define STREAM_TYPE_AUDIO_EAC3      0x87
	// service_type values as defined in ETSI 300 468
	enum {
		MPEGTS_SERVICE_TYPE_DIGITAL_TV = 0x01,
		MPEGTS_SERVICE_TYPE_DIGITAL_RADIO = 0x02,
		MPEGTS_SERVICE_TYPE_TELETEXT = 0x03,
		MPEGTS_SERVICE_TYPE_ADVANCED_CODEC_DIGITAL_RADIO = 0x0A,
		MPEGTS_SERVICE_TYPE_MPEG2_DIGITAL_HDTV = 0x11,
		MPEGTS_SERVICE_TYPE_ADVANCED_CODEC_DIGITAL_SDTV = 0x16,
		MPEGTS_SERVICE_TYPE_ADVANCED_CODEC_DIGITAL_HDTV = 0x19,
		MPEGTS_SERVICE_TYPE_HEVC_DIGITAL_HDTV = 0x1F,
	};
	/* mpegts section writer */

	  struct cmpeg_ts_section {
		int32 pid;
		int32 cc;
		int32 discontinuity;
		//void (*write_packet)(struct MpegTSSection* s, const uint8_t* packet);
		void* opaque;
		cmpeg_ts_section():pid(0), cc(0), discontinuity(0), opaque(nullptr) {}

	};
}

#endif // _C_MPEGTS_ENCODER_H_