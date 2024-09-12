/***********************************************************************************************
			created: 		2019-05-01

			author:			chensong

			purpose:		ccfg

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
#ifndef _CAV_MEM_POOL_H_
#define _CAV_MEM_POOL_H_
//#include "cconfig.h"
//#include "cconfig.h"
//#include "csingleton.h"
extern "C"
{
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
	//#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/display.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavfilter/buffersink.h>
#include <libavutil/cpu.h>
}



#ifdef _MSC_VER
#if !_DEBUG
#pragma comment(lib, "libavcodec.lib")
#pragma comment(lib, "libavdevice.lib")
#pragma comment(lib, "libavfilter.lib")
#pragma comment(lib, "libavformat.lib")
#pragma comment(lib, "libavutil.lib")
#pragma comment(lib, "libpostproc.lib")
#pragma comment(lib, "libswresample.lib")
#pragma comment(lib, "libswscale.lib")
#else 
#pragma comment(lib, "libavcodecd.lib")
#pragma comment(lib, "libavdeviced.lib")
#pragma comment(lib, "libavfilterd.lib")
#pragma comment(lib, "libavformatd.lib")
#pragma comment(lib, "libavutild.lib")
#pragma comment(lib, "libpostprocd.lib")
#pragma comment(lib, "libswresampled.lib")
#pragma comment(lib, "libswscaled.lib")




#endif // !_DEBUG

#elif defined(__GNUC__)

#else 

#endif 

namespace chen {

	typedef void* (*ObjPoolCBAlloc)(void);
	typedef void  (*ObjPoolCBReset)(void*);
	typedef void  (*ObjPoolCBFree)(void**);
	class cav_mem_pool
	{
	public:
		struct obj_pool
		{
			void* pool[32];
			unsigned int pool_count;

			ObjPoolCBAlloc alloc;
			ObjPoolCBReset reset;
			ObjPoolCBFree  free;
		};
	public:
		explicit cav_mem_pool();

		virtual ~cav_mem_pool();


	public:
		
		obj_pool* alloc(ObjPoolCBAlloc cb_alloc, ObjPoolCBReset cb_reset, ObjPoolCBFree cb_free);


		void free(cav_mem_pool::obj_pool** pop);

		obj_pool* alloc_packets(void);
		obj_pool* alloc_frames(void);

		int  get(obj_pool* op, void** obj);
		void release(obj_pool* op, void** obj);


	public:

	};


}


#endif // _CAV_MEM_POOL_H_