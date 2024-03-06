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
#include "cdecoder.h"
#include "clog.h"
#include "cwebsocket_wan_server.h"
#include "ccfg.h"
namespace chen {


   

	cdecoder::~cdecoder()
	{
	}
    cdecoder* cdecoder::construct()
    {
        return new cdecoder();
    }
    void cdecoder::destroy(cdecoder* ptr)
    {
        delete ptr;
    }
    bool cdecoder::init(uint64 session_id, const char* url)
    {
        destroy();
        std::lock_guard<std::mutex> lock(g_ffmpeg_lock);
         m_session_id = session_id;
        m_url = url;
       
        m_stoped = false;
       
        m_thread = std::thread(&cdecoder::_work_pthread, this);
      


        return true;
    }
    void cdecoder::add_websocket_session(uint64 session_id)
    {
        _send_codec_id(session_id);
        {
            clock_guard lock(m_session_lock); 
            m_session_ids.insert(session_id);
        }
    }
    void cdecoder::delete_websocket_session(uint64 session_id)
    {
        clock_guard lock(m_session_lock);
        m_session_ids.erase(session_id);
    }
    void cdecoder::destroy()
    {
        m_stoped = true;

        if(m_thread.joinable())
        {
            m_thread.join();
        }
        // deactivate interrupt callback
        m_interrupt_metadata.timeout_after_ms = 0;
 



         
        
       
        
        if (m_dict)
        {
            av_dict_free(&m_dict);
            m_dict = NULL;
        }
        
        while (m_packets.size() > 0)
        {
            delete m_packets.front().data;
            m_packets.pop_front();
        }
        if (m_format_ctx_ptr)
        {
            //崩溃问题 下面是释放上下文的步骤顺序
            //::avformat_close_input(&m_format_ctx_ptr);
            ::avformat_free_context(m_format_ctx_ptr); 
            m_format_ctx_ptr = NULL;
        }
        m_session_ids.clear();
       // m_session_id = -1;
        m_stoped = true;
        NORMAL_EX_LOG("decoder --> [url = %s] destroy OK !!!", m_url.c_str());
        //m_open = false;
        //m_pixfmt = AV_PIX_FMT_NONE;
    }
    void cdecoder::all_send_packet()
    {
        std::list<cpacket> listp;
       
        //{
        //    std::lock_guard<std::mutex> lock(m_packet_lock);
        //    if (m_packets.size() > 0)
        //    {
        //        listp = std::move(m_packets);
        //        m_packets.clear();
        //    }
        //}
        //while (listp.size() > 0)
        //{
        //    //listp.front();
        //    //cpacket& packet = listp.front();
        //    g_websocket_wan_server.send_msg(m_session_id, 3, listp.front().data, listp.front().size);
        //    delete listp.front().data;
        //    listp.pop_front();

        //}


    }
    void cdecoder::_send_codec_id(uint64 session_id)
    {
        uint16 codec_id[8];
        // 174 --> H265  
        // 28  --> H264
        if (m_video_codec_id == AV_CODEC_ID_H264)
        {
            codec_id[0] = 28;
            g_websocket_wan_server.send_msg(session_id, 323, &codec_id, sizeof(codec_id));
            NORMAL_EX_LOG("--->>>>>>H264");
        }
        else if (m_video_codec_id == AV_CODEC_ID_HEVC)
        {
            codec_id[0] = 174;
            g_websocket_wan_server.send_msg(session_id, 323, &codec_id, sizeof(codec_id));
        }
    }
    void cdecoder::_send_packet(const AVPacket* packet)
    {
        if (!packet)
        {
            return;
        }
        clock_guard lock(m_session_lock);
        for (const uint64& session  : m_session_ids)
        {
            g_websocket_wan_server.send_msg(session, 3, packet->data, packet->size);
        }
        g_websocket_wan_server.send_msg(m_session_id, 3, packet->data, packet->size);
    }
    void cdecoder::_work_pthread()
    {
        {
            m_open_timeout = g_cfg.get_uint32(ECI_MediaOpenTimeOut)/*LIBAVFORMAT_INTERRUPT_OPEN_DEFAULT_TIMEOUT_MS*/;
            m_read_timeout = g_cfg.get_uint32(ECI_MediaReadTimeOut) /*LIBAVFORMAT_INTERRUPT_READ_DEFAULT_TIMEOUT_MS*/;

            /* interrupt callback */
            m_interrupt_metadata.timeout_after_ms = m_open_timeout;
            get_monotonic_time(&m_interrupt_metadata.value);

            m_format_ctx_ptr = avformat_alloc_context();
            if (!m_format_ctx_ptr)
            {
                WARNING_EX_LOG("alloc avformat context failed !!! url = [%s]", m_url.c_str());
                return  ;
            }
            m_format_ctx_ptr->interrupt_callback.callback = &ffmpeg_util::ffmpeg_interrupt_callback;
            m_format_ctx_ptr->interrupt_callback.opaque = &m_interrupt_metadata;



            const AVInputFormat* input_format = NULL;
            AVDictionaryEntry* entry = av_dict_get(m_dict, "input_format", NULL, 0);
            if (entry != 0)
            {
                input_format = av_find_input_format(entry->value);
            }

            // 1. 打开解封装上下文
            int ret = avformat_open_input(
                &m_format_ctx_ptr, //解封装上下文
                m_url.c_str(),  //文件路径
                input_format, //指定输入格式 h264,h265, 之类的， 传入NULL则自动检测
                &m_dict); //设置参数的字典
            if (ret != 0)
            {
                WARNING_EX_LOG("%s\n", ffmpeg_util::make_error_string(ret));
                return  ;
            }
            //2.读取文件信息
            ret = avformat_find_stream_info(m_format_ctx_ptr, NULL);
            if (ret < 0)
            {
                WARNING_EX_LOG("%s\n", ffmpeg_util::make_error_string(ret));
                return  ;
            }
            //3.获取目标流索引
            for (unsigned int i = 0; i < m_format_ctx_ptr->nb_streams; i++)
            {
                AVStream* stream = m_format_ctx_ptr->streams[i];
                if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    m_video_stream_index = i;
                    m_video_codec_id = stream->codecpar->codec_id;
                    _send_codec_id(m_session_id);
                    //uint16 codec_id[8];
                    //// 174 --> H265  
                    //// 28  --> H264
                    //if (m_video_codec_id == AV_CODEC_ID_H264)
                    //{
                    //    codec_id[0] = 28;
                    //    g_websocket_wan_server.send_msg(m_session_id, 323, &codec_id, sizeof(codec_id));
                    //    NORMAL_EX_LOG("--->>>>>>H264");
                    //}
                    //else if (m_video_codec_id == AV_CODEC_ID_HEVC)
                    //{
                    //    codec_id[0] = 174;
                    //    g_websocket_wan_server.send_msg(m_session_id, 323, &codec_id, sizeof(codec_id));
                    //}
                }
                else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    m_audio_stream_index = i;
                    stream->discard = AVDISCARD_ALL;
                }
                else
                {
                    stream->discard = AVDISCARD_ALL;
                }
            }
        }
        int32_t ret = 0;
        AVPacket* packet_ptr = ::av_packet_alloc();
        while (!m_stoped)
        {
            ::av_packet_unref(packet_ptr);
            get_monotonic_time(&m_interrupt_metadata.value);
            m_interrupt_metadata.timeout_after_ms = m_read_timeout;


            //读取一帧压缩数据
            ret = ::av_read_frame(m_format_ctx_ptr, packet_ptr);
            /*if (ret != AVERROR_EOF && pkt->stream_index != m_video_stream_index)
            {
                av_packet_unref(pkt);
                continue;
            }*/
            if (ret == AVERROR(EAGAIN))
            {
                //continue;
            }
            else if (ret == AVERROR_EOF)
            {
                // flush cached frames from video decoder
             /*   m_packet_ptr->data = NULL;
                m_packet_ptr->size = 0;
                m_packet_ptr->stream_index = m_video_stream_index;*/
            }
            else
            {
                if (packet_ptr->stream_index == m_video_stream_index)
                {
                   // NORMAL_EX_LOG("send video size = [%u]", packet_ptr->size );
                    /*cpacket packet;
                     packet.data = new unsigned char[packet_ptr->size];
                    if (packet.data)
                    {
                         
                        memcpy(packet.data, packet_ptr->data, packet_ptr->size);
                        packet.size = packet_ptr->size;
                        {
                            std::lock_guard<std::mutex> lock(m_packet_lock);
                            m_packets.push_back(packet);
                        }
                    }
                    else
                    {
                        WARNING_EX_LOG("[url = %s]alloc  pakcet failed !!!", m_url.c_str());
                    }*/
                    _send_packet(packet_ptr);
                  //  packet_ptr->data
                    ::av_packet_unref(packet_ptr);
                }
                //else
                {
                    ::av_packet_unref(packet_ptr);
                }
            }
           // NORMAL_EX_LOG("read_frame_packet ret = [%u]", ret);
        }
        WARNING_EX_LOG("[url = %s][m_stoped = %u]", m_url.c_str(), m_stoped);
    }
}