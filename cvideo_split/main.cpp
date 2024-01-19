#include <iostream>
#include <signal.h>
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

	m_stoped = true;
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



	 




	for (int h = y; (h < y + dst_height) && (h < src_height); ++h)
	{
		
		memcpy((*dst_data) + (((h - y) *   ((dst_width / 2) + dst_width))),
			src_y + (h * src_width) + x, dst_width);
		memcpy((*dst_data) + (((h - y) * ((dst_width / 2) + dst_width) + dst_width)  ),
			src_u + (((h - y) * (src_width / 4))  ), dst_width/4);
		memcpy((*dst_data) + (((h - y) * ((dst_width / 2) + dst_width) + dst_width + (dst_width/4))),
			src_v + (((h - y) * (src_width / 4))  ), dst_width / 4);
		//memcpy((*dst_data) + (((h - y) * dst_width) + (dst_width / 2) * (h - y) / 2) + dst_width/2 + dst_width, src_v + ((h/2) * src_width /2) + x / 2, dst_width / 2);

	}

	return true;
}

int main(int argc, char* argv[])
{
	//RegisterSignal();
	//open();
	//cv::resizeWindow(m_source_path, 800, 600);
	//decode();
	chen::cdecode decode;

	if (!decode.init(m_source_path.c_str()))
	{
		printf("[decodec init = = %s]failed !!!\n", m_source_path.c_str());
		return false;
	}
	AVFrame* frame = NULL;
	FILE* out_file_ptr = ::fopen("./chensong.yuv", "wb+");
	int x = 100;
	int y = 100;
	int width = 800;
	int height = 800;
	unsigned char* buffer = reinterpret_cast<unsigned char *>(malloc(sizeof(unsigned char ) * width * height * 4));
	while (true)
	{
	retry:
		
		int ret = decode.retrieve(frame);
		if (ret < 0)
		{
			throw;
		}
		//读取文件结束位置了
		if (ret == 0)
		{
			decode.seek(0);
			goto retry;
		}
		mat_copy_rect(frame->data[0], frame->data[1], frame->data[2], decode.get_width(), decode.get_height(), x, y, &buffer, width, height);
		//::fwrite(frame->data[0], 1, decode.get_width() * decode.get_height(), out_file_ptr);
		// ::fwrite(frame->data[1], 1, decode.get_width() * decode.get_height()/4, out_file_ptr);
		// ::fwrite(frame->data[2], 1, decode.get_width() * decode.get_height()/4, out_file_ptr;
		 ::fwrite(buffer, 1, (width * height)/2 + (width * height), out_file_ptr);
		 ::fflush(out_file_ptr);
	}


	return EXIT_SUCCESS;
}