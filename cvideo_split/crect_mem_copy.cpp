/***********************************************************************************************
created: 		2024-01-21

author:			chensong

purpose:		rect_mem_copy

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


#include "crect_mem_copy.h"
#include <cstdio>
#include <cstdlib>
#include "cdecode.h"
#include <cstring>
#include <string>
namespace chen {
	bool rect_mem_copy(const unsigned char* data, int src_width, int src_height,
		unsigned char** rect_data, int rect_width, int rect_height,
		int src_x, int src_y, int rect_x, int rect_y, int dst_width, int dst_height, EFormatType type)
	{
	
		int new_rect_h = rect_y;

		for (size_t h = src_y; (h < src_height && (h - src_y < dst_height)); ++h)
		{
			// cur 
			// 从第几行开始
			int new_h = h - src_y;
			int hh = new_rect_h + new_h;
			/*::printf("[h = %u]\n", new_h);
			::fflush(stdout);*/
			memcpy(((*rect_data) + (hh * rect_width) + rect_x) /*计算新的*/,
				(data + ((h * src_width)) + src_x)/*实际数据的拷贝*/,
				dst_width);
		}
		return true;
		return true;
	}
	void test_rect_mem_copy(const char* input_h264, const char* out_file_yuv)
	{
		chen::cdecode decode;
		 
		if (!decode.init(input_h264))
		{
			printf("[decodec init = = %s]failed !!!\n", input_h264);
			return ;
		}
		AVFrame* frame = NULL;
		FILE* out_file_ptr = ::fopen(out_file_yuv, "wb+");
		 
		//当前画面开始位置的坐标拷贝
		int x = 400; // 
		int y = 400;
		// 需要拷贝画面的宽度和高度
		int width = 400;
		int height = 400;
		// 需要拷贝到新的画面矩阵的宽度和高度
		int rect_width = 1600;
		int rect_height = 880;
		// 需要拷贝到新的画面位置开始位置的坐标
		int rect_x = 200;
		int rect_y = 200;

		unsigned char* buffer = reinterpret_cast<unsigned char*>(malloc(sizeof(unsigned char) * rect_width * rect_height * 4));
		while (true)
		{
		 
			int ret = decode.retrieve(frame);
			if (ret < 0)
			{
				break;
				
			}
			//读取文件结束位置了
			if (ret == 0)
			{
				break; 
			}
			 
			 
			{ 
				rect_mem_copy(frame->data[0], decode.get_width(), decode.get_height(),
					&buffer, rect_width, rect_height, x, y, rect_x, rect_y, width, height, EFormatYuv420P);
				 
				unsigned char* u_ptr = buffer + (rect_width * rect_height);
				rect_mem_copy(frame->data[1], decode.get_width() / 2, decode.get_height() / 2,
					&u_ptr, rect_width / 2, rect_height / 2, x / 2, y / 2, rect_x / 2, rect_y / 2, width / 2, height / 2
					, EFormatYuv420P);

				unsigned char* v_ptr = buffer + (rect_width * rect_height) + (rect_width * rect_height / 4);
				rect_mem_copy(frame->data[2], decode.get_width() / 2, decode.get_height() / 2,
					&v_ptr, rect_width / 2, rect_height / 2, x / 2, y / 2, rect_x / 2, rect_y / 2, width / 2, height / 2
					, EFormatYuv420P);

			}
			 
			::fwrite(buffer, 1, (((rect_width * rect_height) / 2) + (rect_width * rect_height)), out_file_ptr);
			::fflush(out_file_ptr);
		}
		decode.destroy();
		if (frame)
		{
			::av_frame_free(&frame);
			frame = NULL;
		}
		if (buffer)
		{
			::free(buffer);
			buffer = NULL;
		}
		if (out_file_ptr)
		{
			::fclose(out_file_ptr);
			out_file_ptr = NULL;
		}
		 
	}
}

