//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <signal.h>
#include "crect_mem_copy.h"
//#include <opencv2/core.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/dnn/dnn.hpp>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv2/cudaimgproc.hpp>
//#include <opencv2/cudacodec.hpp>
#include "cdecode.h"
#include <cstdlib>
#include "cencoder.h"
#include "XlsReader.h"
#include "cglobal_ffmpeg_register.h"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/util/type_resolver_util.h>
#include "VideoSplit.pb.h"
#include "cvideo_split_server.h"
#include <json/json.h>
void test_protobuf_json()
{
	// ... 你的protobuf消息定义 ...

	AddCameraInfos message;
	// 填充message对象

	std::string json_output;
	google::protobuf::util:: JsonOptions options;
	//google::protobuf::util::MessageToJsonString(message, &json_output, options);
}
//#pragma comment(lib, "opencv_world460.lib")
//#pragma comment(lib, "opencv_img_hash460.lib")
//#pragma comment(lib, "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cudart_static.lib")


//#pragma comment(lib, "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cufft.lib")
//#pragma comment(lib, "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\curand.lib")
//#pragma comment(lib, "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cublas.lib")
//#pragma comment(lib, "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cublasLt.lib")
//#pragma comment(lib, "C:\Program Files\NVIDIA Corporation\NvToolsExt\\lib\x64\nvToolsExt64_1.lib")
/*
C:\Program Files\NVIDIA Corporation\NvToolsExt\\lib\x64\nvToolsExt64_1.lib
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cudart_static.lib
C:\Program Files\NVIDIA Corporation\NvToolsExt\lib\x64\nvToolsExt64_1.lib
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cufft.lib
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\curand.lib
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cublas.lib
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64\cublasLt.lib
*/

# 
//cv::VideoCapture* m_video_cap_ptr;
std::string m_source_path = "D:/Tools/input.h264";
int32_t						m_video_index;
bool					m_stoped = false;

void Stop(int i)
{

	//m_stoped = true;
	chen::g_video_split_server.stop();
}

void RegisterSignal()
{
	signal(SIGINT, Stop);
	signal(SIGTERM, Stop);

}
//bool open()
//{
//	if (m_stoped)
//	{
//		return false;
//	}
//	m_video_cap_ptr = new cv::VideoCapture();
//	//m_video_cap_ptr->set();
//	m_video_cap_ptr->open(m_source_path, cv::CAP_FFMPEG);
//	if (!m_video_cap_ptr->isOpened())
//	{
//		std::cerr << "ERROR! Can't to open file: " << m_source_path << std::endl;
//		//WARNING_EX_LOG("Can't ot open [source = %s]", m_source_path.c_str());
//		return false;
//	}
//	//m_source_path = source;
//	//const int videoBaseIndex = (int)m_video_cap_ptr->get(cv::CAP_PROP_VIDEO_STREAM);
//	m_video_index = (int)m_video_cap_ptr->get(cv::CAP_PROP_VIDEO_TOTAL_CHANNELS);
//	return true;
//}
//void decode()
//{
//	std::chrono::steady_clock::time_point cur_time_ms;
//	std::chrono::steady_clock::time_point pre_time = std::chrono::steady_clock::now();
//	std::chrono::steady_clock::duration dur;
//	std::chrono::milliseconds ms;
//	uint32_t elapse = 0;
//	uint32_t total_ms = 100 / 25;
//
//	cv::Mat img;
//
//
//	while (!m_stoped)
//	{
//		pre_time = std::chrono::high_resolution_clock::now();
//		if (m_video_index != -1 && m_video_cap_ptr->grab() 
//			&& m_video_cap_ptr->retrieve(img, m_video_index) 
//			/*d_reader->grab() && d_reader->nextFrame(d_frame)*/)
//		{
//			/*if (++skip_frame_count > m_skip_frame)
//			{
//				skip_frame_count = 0;
//				{
//					std::lock_guard<std::mutex> lock(m_queue_lock);
//					m_queue.push_back(img.clone());
//
//				}
//			}*/
//			//cv::imshow(m_source_path, img);
//			//cv::imwrite("test.jpg", img);
//			//cv::waitKey(1);
//			/*std::string s = "DSFDEGETEFDSFDSFDSFDSSFDSFEFEFEFDSFDSFDSFDSFEFEFEFG";
//			cv::HersheyFonts font_face = cv::FONT_HERSHEY_DUPLEX;
//			double font_scale = 1.0;
//			int thickness = 1;
//			int baseline = 0;
//			cv::Size s_size = cv::getTextSize(s, font_face, font_scale, thickness, &baseline);
//			cv::putText(img, s, cv::Point(100, 100),
//				font_face, font_scale, cv::Scalar(255, 255, 255), thickness);*/
//				// 创建ROI（Region of Interest）对象来表示要提取的区域
//			cv::Rect roi(100, 100, 700, 1000); // x、y为左上角点的坐标，width和height为区域的宽度和高度
//
//			// 从原始图像中提取ROI区域
//			cv::Mat regionOfInterest(img, roi);
//			cv::imshow(m_source_path, regionOfInterest);
//			cv::waitKey(1);
//		}
//		else
//		{
//			//if (!m_video_cap_ptr->isOpened())
//			{
//			//	WARNING_EX_LOG("[source = %s] close !!!  wait %u  s", m_source_path.c_str(), g_cfg.get_uint32(ECI_MeidaReconnectWait));
//			}
//			//m_action = 2;
//			if (!m_stoped)
//			{
//				if (m_video_cap_ptr)
//				{
//					m_video_cap_ptr->release();
//					delete m_video_cap_ptr;
//					m_video_cap_ptr = NULL;
//				}
//				m_video_index = -1;
//				//std::this_thread::sleep_for(std::chrono::seconds(g_cfg.get_uint32(ECI_MeidaReconnectWait)));
//				open();
//				//{
//				//	m_action = 1;
//				//}
//			}
//			//return;
//			continue;
//			// TODO@chensong 20231017 
//
//		}
//		cur_time_ms = std::chrono::steady_clock::now();
//		dur = cur_time_ms - pre_time;
//		ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
//		elapse = static_cast<uint32_t>(ms.count());
//		if (elapse < total_ms && !m_stoped)
//		{
//			std::this_thread::sleep_for(std::chrono::milliseconds(total_ms - elapse));
//		}
//	}
//}
//

bool mat_copy_rect(const unsigned char * src_y, const unsigned char* src_u, const unsigned char* src_v,
	int src_width, int src_height, int x, int y, unsigned char ** dst_data, int dst_width, int dst_height)
{



	 /*
	 Y数据， 分辨率: 1920x1080
	 U数据， 分辨率: 960x540
	 V数据， 分辨率: 960x540
	 */




	for (int h = y; (h <= y + dst_height) && (h < src_height); ++h)
	{
		
		memcpy((*dst_data) + (((h - y) *   (   + dst_width))),
			src_y + ((h * src_width) + x), dst_width);



		if (h >= (src_height / 2))
		{
			continue;
		}
		//int uv_width = 
		// memcpy((*dst_data) +  ((dst_height * dst_width) + ((h-y) * (dst_width/2))),
		//	src_u + (((h - y) * (src_width / 2)) + (x/4) ), dst_width/2);
 		//
		//
		//
		//
		// memcpy((*dst_data) + ((dst_height * dst_width) +(dst_width * dst_height /4) + ((h - y) * (dst_width / 2))),
		//	src_v + (((h - y) * (src_width / 2))+ (x/4)  ), dst_width / 2);
		// //memcpy((*dst_data) + (((h - y) * dst_width) + (dst_width / 2) * (h - y) / 2) + dst_width/2 + dst_width, src_v + ((h/2) * src_width /2) + x / 2, dst_width / 2);

	}

	return true;
}
/*
* 
*/
bool mat_copy_rect_new_rect(const unsigned char* data, int src_width, int src_height,
	unsigned char** rect_data, int rect_width, int rect_height,
	int src_x, int src_y, int rect_x, int rect_y, int dst_width, int dst_height )
{

	int new_rect_h = rect_y;

	for (size_t h = src_y; (h < src_height && (h - src_y < dst_height)  ); ++h)
	{
		// cur 
		// 从第几行开始
		int new_h =   h - src_y;
		int hh  = new_rect_h + new_h;
		/*::printf("[h = %u]\n", new_h);
		::fflush(stdout);*/
		memcpy(((*rect_data) + (hh * rect_width) + rect_x) /*计算新的*/,
			(data + ((h * src_width)) + src_x )/*实际数据的拷贝*/,
			dst_width);
	}
	return true;
}
int test_main(int argc, char* argv[])
{
	 
	chen::test_rect_mem_copy(m_source_path.c_str(), "chensong.yuv");

	return EXIT_SUCCESS;
}


void test_json()
{
	Json::Value value;
	/*value["result"] = result;
	value["data"] = data;
	value["message"] = message ? message : "";*/
	for (size_t i = 0; i < 3; ++i)
	{
		Json::Value Camera;
		Camera["addres"] = "192.168.1.2" + std::to_string(i);
		//value.append(Camera);
		value.append( Camera);
	}
	Json::StyledWriter swriter;
	std::string str = swriter.write(value);
	printf("[camera_list = %s]\n", str.c_str());
}
<<<<<<< HEAD
#include "cglobal_ffmpeg_register.h"
//#include <opencv2/opencv.hpp>
#include "cavfilter_video.h"

void test_avfilter_demo()
{
	chen::CVideoMerge cVideoMerge;
	const char *pFileA = "D:/Tools/input.h264";
	const char *pFileB = "D:/Tools/input.h264";

	const char *pFileOut = "chensong.mp4";
	cVideoMerge.StartMerge(pFileA, pFileB, pFileOut);
	cVideoMerge.WaitFinish();
	 
}
 

std::string core_name(const std::string& path)
{ 
	test_avfilter_demo();
	return 0;
	printf("Hello video mark!\n");
	int ret = 0;
	FILE* infile = NULL;
	const char* infileName = "chensong.yuv";
	fopen_s(&infile, infileName, "rb");
	if (!infile) 
	uint32_t index = 0;
	for (uint32_t i = 0; i < path.length(); ++i) 
	{
		if (path.at(i) == '/' || path.at(i) == '\\')
		{
			index = i;
		}
	}
	return path.substr(index+1, path.length());
}
int  main(int argc, char** argv) 
{

	 
	RegisterSignal();
	const char* config_filename = "server.cfg";
	const char* log_path = "./log";
	if (argc > 1)
	{
		config_filename = argv[1];
	}
	if (argc > 2)
	{
		log_path = argv[2];
	}  
	bool init = chen::g_video_split_server.init(log_path, config_filename);

	if (init)
	{
		init = chen::g_video_split_server.Loop();
	}
	chen::g_video_split_server.Destroy();
	if (!init)
	{
		return 1;
	}





	return 0;
	test_json();
	return 0;





	//::setlocale(LC_ALL, "chs");
	xls::WorkBook test("test.xls");

	xls::cellContent cell = test.GetCell(0, 1, 11 );
	test.ShowCell(cell);

	for (int sheetNum = 0; sheetNum < test.GetSheetCount(); ++sheetNum) 
	{
		if (sheetNum)
		{
			printf("\n\n");
		}
		printf("===>>> SheetName: %s\n\n", test.GetSheetName(sheetNum).c_str());

		test.InitIterator(sheetNum);
		while (true) 
		{
			xls::cellContent c = test.GetNextCell();
			if (c.type == xls::cellBlank)
			{
				break;
			}
			test.ShowCell(c);
		}
	}
	return 0;
//>>>>>>> c01615f5939020980b763ea04154e8ee4c9065a4
	//****初始化,分配内存,声明参数****//
	
	//avformat_network_init();
	//avcodec_register_all();
//<<<<<<< HEAD
//=======


	chen::g_global_ffmpeg_register.init();

	chen::cdecode decode1;
	chen::cdecode decode2;





	AVFilterGraph* g_graph = NULL;
	AVFilterContext* g_buffer1_ctx = NULL; 
	AVFilterContext* g_buffer2_ctx = NULL;
	AVFilterContext* g_buffer1_crop_ctx = NULL;
	AVFilterContext* g_buffer2_crop_ctx = NULL;
	AVFilterContext* g_buffer1_scale_ctx = NULL;
	AVFilterContext* g_buffer2_scale_ctx = NULL;
	AVFilterContext* g_hstack_ctx = NULL;


	AVFilterContext* g_buffersink_ctx = NULL; 




	// udp://@224.1.2.3:20000
	// // http://192.168.2.192/
	// http://192.168.2.192/cgi-bin/configManager.cgi?action=getConfig&name=Multicast
	// http://192.168.2.192/cgi-bin/configManager.cgi?action=setConfig&Multicast.TS[3].Enable=true
	if (!decode1.init("udp://@224.1.1.3:20000"))
	{
		printf("[decodec init = = %s]failed !!!\n", "udp://@224.1.2.3:20048");
		return 0;
	}

	{

	


	
	}//udp://@224.1.1.3:20000
	if (!decode2.init("udp://@224.1.1.3:20000"))
	{
		printf("[decodec init = = %s]failed !!!\n", "udp://@224.1.2.3:20048");
		return 0;
	}

	// init filter 
	{
		int32_t ret = 0;
		//;[v0][v1]hstack=2[out]
		//[0:v]crop=1900:800:100:100,scale=1920:1080[v0];[1:v]crop=1900:800:10:10,scale=1920:1080[v1];[v0][v1]hstack=2[out]
		//const char* filter_desc = "crop=1900:800:100:100,scale=1920:1080";
		const char* filter_desc = "[0:v]crop=1900:800:10:10,scale=1920:1080[v0];[1:v]crop=1900:800:10:10,scale=1920:1080[v1];[v0][v1]hstack=2[out]";
		//const char* filter_desc = "[in1]crop=1900:800:100:100,scale=1920:1080[in0];[in1]crop=1900:800:10:10,scale=1920:1080[in1];[in0][in1]hstack=2[out]";

		
		
		const AVFilter* buffersrc1 = ::avfilter_get_by_name("buffer");
		const AVFilter* buffersrc2 = ::avfilter_get_by_name("buffer");
		const AVFilter* bufcrop1_src = avfilter_get_by_name("crop");
		const AVFilter* bufcrop2_src = avfilter_get_by_name("crop");
		const AVFilter* bufscale1_src = avfilter_get_by_name("scale"); 
		const AVFilter* bufscale2_src = avfilter_get_by_name("scale"); 
		const AVFilter* hstack = avfilter_get_by_name("hstack");
		
		const AVFilter* buffersink = ::avfilter_get_by_name("buffersink");


		AVFilterInOut* inputs = ::avfilter_inout_alloc(); 
		AVFilterInOut* outputs = ::avfilter_inout_alloc(); 
		if (!inputs)
		{
			printf("alloc inputs failed !!!\n");
			return -1;
		}
		if (!outputs)
		{
			printf("alloc outputs failed !!!\n");
			return -1;
		}

		char args[1024] = { 0 };
		AVRational time_base = decode1.m_ic_ptr ->streams[decode1.m_video_stream_index]->time_base;
		// video_size=wxh:pix_fmt=xx:time_base=xxx/xxxx:pixel_aspect=xxx/xx
		snprintf(args, 1024, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			decode1.m_codec_ctx_ptr->width, decode1.m_codec_ctx_ptr->height, decode1.m_codec_ctx_ptr->pix_fmt,
			time_base.num, time_base.den,
			decode1.m_codec_ctx_ptr->sample_aspect_ratio.num, decode1.m_codec_ctx_ptr->sample_aspect_ratio.den);

		//
		g_graph = ::avfilter_graph_alloc();

		if (!g_graph)
		{
			printf("alloc graph failed !!!\n");
			return -1;
		}

		//解析描述符 filter_desc 
		// "[0:v]crop=900:900:10:10,scale=1920:1080[v0];[1:v]crop=900:900:10:10,scale=1920:1080[v1];[v0][v1]hstack=2[out]"
		// [a][b]overlay=xxxx[out]
		// 输入 buffer filter 创建
		// 构造input 和output
		// args : 查看参数 命令 : ./ffmpeg -h filter=buffer
		// src buffer 0 
		ret = ::avfilter_graph_create_filter(&g_buffer1_ctx, buffersrc1, "in0", args, NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}

		/*snprintf(args, 1024, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			decode1.m_codec_ctx_ptr->width, decode1.m_codec_ctx_ptr->height, decode1.m_codec_ctx_ptr->pix_fmt,
			time_base.num, time_base.den,
			decode1.m_codec_ctx_ptr->sample_aspect_ratio.num, decode1.m_codec_ctx_ptr->sample_aspect_ratio.den);
	*/	 
		std::string crop_str1 = "1910:1000:10:10";
		ret = ::avfilter_graph_create_filter(&g_buffer1_crop_ctx, bufcrop1_src, "crop", crop_str1.c_str(), NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}
		std::string scale_str1 = "1920:1080";
		ret = ::avfilter_graph_create_filter(&g_buffer1_scale_ctx, bufscale1_src, "scale", scale_str1.c_str(), NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}
		 
		// src 1
		 snprintf(args, 1024, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			decode1.m_codec_ctx_ptr->width, decode1.m_codec_ctx_ptr->height, decode1.m_codec_ctx_ptr->pix_fmt,
			time_base.num, time_base.den,
			decode1.m_codec_ctx_ptr->sample_aspect_ratio.num, decode1.m_codec_ctx_ptr->sample_aspect_ratio.den);
	 
		ret = ::avfilter_graph_create_filter(&g_buffer2_ctx, buffersrc2, "in1", args, NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}

		
		std::string crop_str2 = "1920:1000:10:10";
		ret = ::avfilter_graph_create_filter(&g_buffer2_crop_ctx, bufcrop2_src, "crop", crop_str2.c_str(), NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}
		std::string scale_str2 = "1920:1080";
		ret = ::avfilter_graph_create_filter(&g_buffer2_scale_ctx, bufscale2_src, "scale", scale_str2.c_str(), NULL/*buffer 用户数据*/, g_graph);
		if (ret < 0)
		{
			printf("in -->avfilter graph create filter failed !!!");
			return ret;
		}

		/////   hstatck 
		ret = avfilter_graph_create_filter(&g_hstack_ctx, hstack, "hstack", "inputs=2", NULL, g_graph);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_graph_create_filter  failed! :" << buf << std::endl;
			return false;
		}

		// sink
		ret = avfilter_graph_create_filter(&g_buffersink_ctx, buffersink, "out", NULL, NULL, g_graph);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_graph_create_filter  failed! :" << buf << std::endl;
			return false;
		}

		// 配置  处理流程
		ret = avfilter_link(g_buffer1_ctx, 0, g_buffer1_crop_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
		ret = avfilter_link(g_buffer1_crop_ctx, 0, g_buffer1_scale_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
		ret = avfilter_link(g_buffer1_scale_ctx, 0, g_hstack_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
		ret = avfilter_link(g_buffer2_ctx, 0, g_buffer2_crop_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
		ret = avfilter_link(g_buffer2_crop_ctx, 0, g_buffer2_scale_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
		ret = avfilter_link(g_buffer2_scale_ctx, 0, g_hstack_ctx, 1);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}
//>>>>>>> c01615f5939020980b763ea04154e8ee4c9065a4

		ret = avfilter_link(g_hstack_ctx, 0, g_buffersink_ctx, 0);
		if (0 > ret)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "avfilter_link  failed! :" << buf << std::endl;
			return false;
		}


		//graph 生效
		ret = ::avfilter_graph_config(g_graph, NULL);
		if (ret < 0)
		{
			printf("filter graph config failed (%s)!!!\n", chen::ffmpeg_util::make_error_string(ret));
			return ret;
		}
		if (inputs->next)
		{
			::avfilter_inout_free(&inputs->next);
			inputs->next = NULL;
		}
		::avfilter_inout_free(&inputs);
		::avfilter_inout_free(&outputs);
		inputs  = NULL;
		 
		outputs = NULL;
		 
	}
	 
	chen::cencoder encoder;
	if (!encoder.init( "udp://@239.255.255.250:54540", 3840, 1080  ))
	{
		encoder.stop();
		
	}
	 
	AVFrame* frame = NULL;
	int32_t sleep_ms = 1000 / 28;


	uint8_t* data = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * (decode1.get_height() * decode1.get_width() * 4)));;
	int32_t width = 0;
	int32_t height = 0;
	int32_t step = 0;
	int32_t cn = 0;
	int32_t count = 0;
	 
	AVFrame* filter_frame = av_frame_alloc();
	AVFrame* frame_ptr = NULL;;
	while (true)
	{
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		
		{
			bool ret = decode1.retrieve(frame_ptr
			/*&data, &step, &width, &height, &cn*/);
			if (!ret)
			{
				//des:
				++count;
				//decode.destroy();
				printf("-->>>>>>>>>>>>>>>>decode1>>>>>>>>>>>>>>\n");
				if (count > 3)
				{
					decode1.destroy();
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
					decode1.init("udp://@224.1.1.3:20000");
					count = 0;
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
				}
				continue;

			}
			  ret = ::av_buffersrc_add_frame(g_buffer1_ctx, frame_ptr);
			if (ret < 0)
			{
				printf("filter buffer1src add frame failed (%s)!!!\n", chen::ffmpeg_util::make_error_string(ret));
				//return ret;
			}
			::av_frame_unref(frame_ptr);
		//	encoder.consume_frame1(frame_ptr/*data, step, width, height, cn*/);
		}
		{
			bool ret = decode2.retrieve(  frame_ptr 
				/*&data, &step, &width, &height, &cn*/);
			if (!ret)
			{
				//des:
				++count;
				//decode.destroy();
				printf("-->>>>>>>>>>>>>>>>decode2>>>>>>>>>>>>>>\n");


				if (count > 3)
				{
					decode2.destroy();
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
					decode2.init("udp://@224.1.1.3:20000");
					count = 0;
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(30));
				}
				continue;

			}
			  ret = ::av_buffersrc_add_frame(g_buffer2_ctx, frame_ptr);
			if (ret < 0)
			{
				printf("filter buffer2src add frame failed (%s)!!!\n", chen::ffmpeg_util::make_error_string(ret));
				//return ret;
			}
			::av_frame_unref(frame_ptr);
			ret = ::av_buffersink_get_frame(g_buffersink_ctx, filter_frame);
			if (ret < 0)
			{
				//有可能buuffersink还没有处理好数据 需要继续获取得啦 ~~~~
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					// 需要释放资源啦 ~~~~
					// av_frame_unref(filter_frame);
					break;
				}
				printf("avfilter failed !!!\n");
				continue;
//				return ret;
			}
			printf("[width = %u][height = %u]\n", filter_frame->width, filter_frame->height);
		 	encoder.push_frame(filter_frame);
			::av_frame_unref(filter_frame);
			// 
			//::av_frame_unref(filter_frame);
			//encoder.consume_frame2(frame_ptr/* data, step, width, height, cn*/);
		}
		std::chrono::milliseconds cur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds diff_ms = cur_ms - ms;
		//printf(" [frame = %u][%u][%u] ms [pts = %u]\n", frame_count, ::time(NULL), diff_ms.count(), m_pkt_ptr->dts);
		ms = cur_ms;
		printf("diff [%lu]ms[cpu = %u]\n", diff_ms.count(), av_cpu_count());
		if (sleep_ms > diff_ms.count())
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms - diff_ms.count()));
		}
	}
	decode1.destroy();
	decode2.destroy();
	encoder.stop();
	encoder.destroy();
	if (frame)
	{
		::av_frame_free(&frame);
		frame = NULL;
	}
	/*while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}*/

	//avcodec_register_all();
	return EXIT_SUCCESS;
	AVPacket* decodePacket;
	AVPacket *encodePacket;
	//av_init_packet(&decodePacket);
	decodePacket = av_packet_alloc();
	const AVCodec* pDecodec = NULL;
	const AVCodec* pEncodec = NULL;

	AVFrame* pInFrame = av_frame_alloc();
	AVFrame* pOutFrame = av_frame_alloc();

	AVFormatContext* pInFormatContext = avformat_alloc_context();
	AVFormatContext* pOutFormatContext = avformat_alloc_context();

	AVInputFormat* pInFormat = NULL;
	AVOutputFormat* pOutFormat = NULL;

	AVStream* pInStream = NULL;
	AVStream* pOutStream = NULL;

	//****设置参数****//
	//udp://@224.1.2.3:20000
	const char* inFileName = "udp://@224.1.2.3:20048"; //这个只有视频轨,没有音轨
	const char* outFileName = "udp://239.255.255.250:54546"; //nginx 流服务器地址

	//****打开输入流****//
	int ret = avformat_open_input(&pInFormatContext, inFileName, NULL, NULL); //打开上下文,上下文的内存在前面分配
	if (ret < 0) {
		printf("Open input formatcontext failed,%d\n", ret);
		return 1;
	}
	if ((ret = avformat_find_stream_info(pInFormatContext, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		return 1;
	}

	av_dump_format(pInFormatContext, 0, inFileName, 0); //显示一下
	pInStream = pInFormatContext->streams[0]; //赋值

	pDecodec = avcodec_find_decoder(pInStream->codecpar->codec_id); //找到解码器
	if (!pDecodec) {
		printf("Find decoder failed\n");
		return 1;
	}
	AVCodecContext * decoder_context = avcodec_alloc_context3(pDecodec);
	ret = avcodec_open2(decoder_context, pDecodec, NULL); //打开输入流
	if (ret < 0) {
		printf("Open instream failed,%d\n", ret);
		return 1;
	}

	//****计算输入流信息****//
	AVRational frame_rate;
	double duration;
	frame_rate = av_guess_frame_rate(pInFormatContext, pInStream, NULL); //获取码率
	// frame_rate = {25,1};
	duration = (frame_rate.num && frame_rate.den ? av_q2d(frame_rate) : 0); //计算延时

	//****分配输出上下文****//
	bool push_stream = false;
	std::string outFormatName ;
	if (strstr(outFileName, "rtmp://") != NULL) {
		push_stream = true;
		outFormatName = "flv";
	}
	else if (strstr(outFileName, "udp://") != NULL) {
		push_stream = true;
		outFormatName = "mpegts";
	}
	else {
		push_stream = false;
		outFormatName.clear();
	}
	avformat_alloc_output_context2(&pOutFormatContext, NULL, outFormatName.c_str(), outFileName);

	//****创建输出流****//
	pEncodec = avcodec_find_encoder(pInStream->codecpar->codec_id); //找到编码器
	AVCodecContext* pEncodeContext = avcodec_alloc_context3(pEncodec); //分配编码上下文
	// pEncodeContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY;
	//av_opt_set(pEncodeContext->priv_data, "tune", "zerolatency", 10);
	//一组图片的数量
	pEncodeContext->gop_size = 30;
	//去掉B帧只留下 I帧和P帧
	pEncodeContext->max_b_frames = 0;

	pOutStream = avformat_new_stream(pOutFormatContext, NULL); //分配流空间

	//设置输出流的参数
	//编码器参数
	pEncodeContext->width = pInStream->codecpar->width;
	pEncodeContext->height = pInStream->codecpar->height;
	pEncodeContext->sample_aspect_ratio = pInStream->codecpar->sample_aspect_ratio; //采样率
	// if (pEncodec->pix_fmts){ // 编码器支持的像素格式列表
	// pEncodeContext->pix_fmt = pEncodec->pix_fmts[0]; // 编码器采用所支持的第一种像素格式
	// }
	// else{
	pEncodeContext->pix_fmt = (AVPixelFormat)pInStream->codecpar->format; // 编码器采用解码器的像素格式
	// }
	//pEncodeContext->time_base = decoder_context->time_base;
	pEncodeContext->time_base = av_inv_q(pInStream->avg_frame_rate); // 时基：解码器帧率取倒数
	pEncodeContext->framerate = decoder_context->framerate;
	  pEncodeContext->time_base.num *= 3.0;
	  pEncodeContext->framerate.den *= 3.0;
	if (pOutFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pEncodeContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	//AVDictionary *opts;
	//av_dict_set(&opts, "b", "2.5M", 0);
	ret = avcodec_open2(pEncodeContext, pEncodec, NULL);
	if (ret < 0) {
		printf("打开编码器");
		return 1;
	}
	ret = avcodec_parameters_from_context(pOutFormatContext->streams[0]->codecpar, pEncodeContext);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", ret);
		return ret;
	}
	// 拷贝输入流信息到输出流
	// ret = avcodec_parameters_copy(pOutStream->codecpar, pInStream->codecpar);
	// if (ret < 0) {
	// printf("copy 编解码器上下文失败\n");
	// }
	// pOutStream->codecpar->codec_tag = 0;
	// ret = avcodec_open2(pOutStream->codec,pEncodec,NULL); // 打开输出流
	// if(ret < 0){
	// printf("Open output stream faild,%d\n",ret);
	// }
	pOutStream = pOutFormatContext->streams[0];

	//****分配推流io上下文****//
	ret = avio_open(&pOutFormatContext->pb, outFileName, AVIO_FLAG_WRITE);
	if (ret < 0) {
		printf("Could not open output file '%s'\n", outFileName);
		return -1;
	}

	ret = avformat_write_header(pOutFormatContext, NULL); //写入头
	// pOutFormatContext->streams[0]->time_base.num *= 10;
	if (ret < 0) {
		printf("Error occurred when opening output file\n");
		return -1;
	}

	//****处理主流程****//
	int gotPicture = 0;
	int index = 0;
	while (av_read_frame(pInFormatContext,  decodePacket) >= 0) {
		// gotPicture = 0;
		// ret = avcodec_decode_video2(pInStream->codec, pInFrame, &gotPicture, &decodePacket); //旧接口,解码
		ret = avcodec_send_packet(decoder_context,  decodePacket);
		ret = avcodec_receive_frame(decoder_context, pInFrame);
		if (ret == -11) {
			continue;
		}
		if (ret < 0) {
			printf("decode error,%d\n", ret);
			break;
		}
		// if(!pInFrame){
		if (pInFrame) {
			// //自定义处理代码
			// //**//


			static FILE* out_file_ptr = fopen("chensong.yuv", "wb+");
			if (out_file_ptr)
			{
				fwrite(pInFrame->data[0], 1, pInFrame->width* pInFrame->height, out_file_ptr);
				fwrite(pInFrame->data[1], 1, pInFrame->width* pInFrame->height /4, out_file_ptr);
				fwrite(pInFrame->data[2], 1, pInFrame->width* pInFrame->height/ 4, out_file_ptr);
				fflush(out_file_ptr);
			}

			encodePacket = av_packet_alloc();
			//av_init_packet(&encodePacket);
			//encodePacket.data = NULL;
			//encodePacket.size = 0;

			// ret = avcodec_encode_video2(pEncodeContext,&encodePacket,pInFrame,&gotPicture);
			ret = avcodec_send_frame(pEncodeContext, pInFrame);
			ret = avcodec_receive_packet(pEncodeContext,  encodePacket);
			if (ret == -11) {
				// printf("encode wrong\n");
				continue;
			}
			// 重新打时间戳
			encodePacket->pts = decodePacket->pts;// + (int)(duration*AV_TIME_BASE);
			encodePacket->dts = decodePacket->dts;// + (int)(duration*AV_TIME_BASE);
			encodePacket->stream_index = 0;

			printf("pts:%ld , dts:%ld , duration*AV_TIME_BASE:%d\n", 
				encodePacket->pts, encodePacket->dts, (int)(duration * AV_TIME_BASE));
			av_packet_rescale_ts( encodePacket, pInStream->time_base, pOutStream->time_base);
			encodePacket->pos = -1;
			// 4. 将编码后的packet写入输出媒体文件
			ret = av_interleaved_write_frame(pOutFormatContext,  encodePacket);
			// av_packet_unref(&encodePacket);

		}
		else {
			// av_usleep((int64_t)(duration*AV_TIME_BASE)); //延时 模拟处理 以后似乎需要用处理流程取减
			decodePacket->stream_index = 0;
			//延时 更新packet中的pts和dts
			av_packet_rescale_ts(decodePacket, pInStream->time_base, pOutStream->time_base);
			decodePacket->pos = -1;

			printf("1-%d\n", decodePacket->size);
			//将packet写入输出流 ,推流
			ret = av_interleaved_write_frame(pOutFormatContext,  decodePacket);
			if (ret < 0) {
				printf("Error muxing packet\n");
				break;
			}
		}
		av_packet_unref( decodePacket); //重置流空间
	}
	// 写输出文件尾
	av_write_trailer(pOutFormatContext);

	//****释放内存****//
	av_frame_free(&pInFrame);
	av_frame_free(&pOutFrame);

	avformat_free_context(pInFormatContext);
	avformat_free_context(pOutFormatContext);

	return 0;
}
