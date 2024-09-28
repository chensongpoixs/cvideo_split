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
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "cmpegts_encoder.h"
#include "ch264_stream.h"
#include "clog.h"
#include "ch264_define.h"
#include "cnet_type.h"
#include "cnet_type.h"
#include <cstring>


//#include <WinSock2.h>

extern "C"
{
#include <libavutil/intreadwrite.h>
#include <libavutil/crc.h>
}


namespace chen {
	/*cmpegts_encoder::cmpegts_encoder()
	{
	}*/


    static inline void put16(uint8_t** q_ptr, int val)
    {
        uint8_t* q;
        q = *q_ptr;
        *q++ = val >> 8;
        *q++ = val;
        *q_ptr = q;
    }
    static void putbuf(uint8_t** q_ptr, const uint8_t* buf, size_t len)
    {
        memcpy(*q_ptr, buf, len);
        *q_ptr += len;
    }
	cmpegts_encoder::~cmpegts_encoder()
	{
	}
	bool cmpegts_encoder::init(const char* ip, uint32 port)
	{
        m_ip = ip;
        m_port = port;
		m_m2ts_mode = 0; 

		m_m2ts_video_pid = M2TS_VIDEO_PID;
		m_m2ts_audio_pid = M2TS_AUDIO_START_PID;
		m_m2ts_pgssub_pid = M2TS_PGSSUB_START_PID;
		m_m2ts_textsub_pid = M2TS_TEXTSUB_PID;
		if (m_m2ts_mode) 
		{
			m_pmt_start_pid = M2TS_PMT_PID;
			//if (s->nb_programs > 1) 
			{
				//av_log(s, AV_LOG_ERROR, "Only one program is allowed in m2ts mode!\n");
				//return AVERROR(EINVAL);
			}
		}
		if (m_max_delay < 0) /* Not set by the caller */
		{
			m_max_delay = 0;
		}

		// round up to a whole number of TS packets  
		// TODO de 2930 
		m_pes_payload_size = (m_pes_payload_size + 14 + 183) / 184 * 184 - 14;

		m_pat.pid = PAT_PID;
		/* Initialize at 15 so that it wraps and is equal to 0 for the
		 * first packet we write. */
		m_pat.cc = 15;
		//m_pat.discontinuity = ts->flags & MPEGTS_FLAG_DISCONT;
		//m_pat.write_packet = section_write_packet;
		m_pat.opaque = this;

		m_sdt.pid = SDT_PID;
		m_sdt.cc = 15;
		//m_sdt.discontinuity = ts->flags & MPEGTS_FLAG_DISCONT;
		//m_sdt.write_packet = section_write_packet;
		m_sdt.opaque = this;

		m_nit.pid = NIT_PID;
		m_nit.cc = 15;
		//m_nit.discontinuity = ts->flags & MPEGTS_FLAG_DISCONT;
		//m_nit.write_packet = section_write_packet;
		m_nit.opaque = this;


        m_socket_fd = ::socket(PF_INET, SOCK_DGRAM, 0);
        //struct sockaddr_in servaddr;
        memset(&m_servaddr, 0, sizeof(m_servaddr));
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_port = htons(m_port);
        m_servaddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_send_buffer = (uint8*)malloc(sizeof(uint8) * 1024 * 3);
        m_send_len = 0;
		/// <summary>
		///   sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        
		/// </summary>
		/// <returns></returns>
        m_stoped = false;

        m_thread = std::thread(&cmpegts_encoder::_pthread_work, this);


		return true;;
	}
	void cmpegts_encoder::update(uint32 DataTime)
	{
	}


    void cmpegts_encoder::stop()
    {
        m_stoped = true;
        m_condition.notify_all();
    
    }
	void cmpegts_encoder::destroy()
	{

        m_stoped = true;
        m_condition.notify_all();
        if (m_thread.joinable())
        {
            m_thread.join();
        }

         
        for (std::list<cmpegts_page>::iterator iter = m_mpegts_page_list.begin(); iter != m_mpegts_page_list.end(); ++iter)
        {
            if (iter->data)
            {
                delete[] iter->data;
                iter->data = NULL;
            }
        }
        m_mpegts_page_list.clear();
        if (m_send_buffer)
        {
            ::free(m_send_buffer);
            m_send_buffer = NULL;
        }
        if (m_socket_fd)
        {
#ifdef _MSC_VER
            ::closesocket(m_socket_fd);
            m_socket_fd = NULL;
#elif defined(__GNUC__) 
            ::close(m_socket_fd);
            m_socket_fd = NULL;
#else
            // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö§ï¿½ÖµÄ±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½Ô¼ï¿½Êµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
#error unexpected c complier (msc/gcc), Need to implement this method for demangle
            return false;
#endif
            
        }
	}
    void cmpegts_encoder::app_push_packet(bool I_frame, uint8* data, uint32 size, int64 pts, int64 dts)
    {
        cmpegts_page page;
        page.I_frame = I_frame;
        page.data = new uint8[size+1];
        memcpy(page.data, data, size);
        page.size = size;
        page.pts = pts;
        page.dts = dts;
        {
            std::lock_guard<std::mutex> lock{ m_mpgets_page_lock };

            m_mpegts_page_list.push_back(page);
        }
        m_condition.notify_one();

    }
	void cmpegts_encoder::push_packet(bool I_frame, uint8* _data, uint32 size, int64 pts, int64 dts)
	{
       // return;
		int32 ret = check_h264_startcode(_data, size);
		if (ret < 0)
		{
			WARNING_EX_LOG("h264 parse start code failed !!!");
			return;
		}
		const uint8* buf = _data;
		const uint8_t* p = buf, * buf_end = buf + size;
		const uint8_t* found_aud = NULL, * found_aud_end = NULL;
		uint32 state = -1;
		uint8_t* data = NULL;
		uint32 extradd = 0;
		int32 payload_size = 0;
		uint8* payload = NULL;
		/* Ensure that all pictures are prefixed with an AUD, and that
		 * IDR pictures are also prefixed with SPS and PPS. SPS and PPS
		 * are assumed to be available in 'extradata' if not found in-band. */
		do {
			p = avpriv_find_start_code(p, buf_end, &state);
			//av_log(s, AV_LOG_TRACE, "nal %"PRId32"\n", state & 0x1f);
			//NORMAL_EX_LOG("[nal = %u]", state & 0X1F);
			if ((state & 0x1f) == H264_NAL_SPS)
			{
				extradd = 0;
			}
			if ((state & 0x1f) == H264_NAL_AUD) 
            {
				found_aud = p - 4;     // start of the 0x000001 start code.
				found_aud_end = p + 1; // first byte past the AUD.
				if (found_aud < buf)
				{
					found_aud = buf;
				}
				if (buf_end < found_aud_end)
				{
					found_aud_end = buf_end;
				}
			}
		} 
        while (p < buf_end
			&& (state & 0x1f) != H264_NAL_IDR_SLICE
			&& (state & 0x1f) != H264_NAL_SLICE
			&& (extradd > 0 || !found_aud));

		if ((state & 0x1f) != H264_NAL_IDR_SLICE)
		{
			extradd = 0;
		}



		if (!found_aud) 
        {
			/* Prefix 'buf' with the missing AUD, and extradata if needed. */
			data = (uint8*)malloc(size + 6 + extradd);
			if (!data)
			{
				WARNING_EX_LOG("alloc failed !!! ");
				return;
			}
				/*return AVERROR(ENOMEM);*/
			//memcpy(data + 6, st->codecpar->extradata, extradd);
			memcpy(data + 6 + extradd, _data, size);
			//data[0] = 0x00;// 000001;
			//data[1] = 0X00;
			//data[2] = 0X00;
			//data[3] = 0X01;
            AV_WB32(data, 0x00000001);

			
			data[4] = H264_NAL_AUD;
			data[5] = 0xf0; // any slice type (0xe) + rbsp stop one bit
			payload = data;
			payload_size = size + 6 + extradd;
		}
		else if (extradd != 0) 
        {
			/* Move the AUD up to the beginning of the frame, where the H.264
			 * spec requires it to appear. Emit the extradata after it. */
			//PutByteContext pb;
			//const int new_pkt_size = pkt->size + 1 + extradd;
			//data = av_malloc(new_pkt_size);
			//if (!data)
			//	return AVERROR(ENOMEM);
			//bytestream2_init_writer(&pb, data, new_pkt_size);
			//bytestream2_put_byte(&pb, 0x00);
			//bytestream2_put_buffer(&pb, found_aud, found_aud_end - found_aud);
			//bytestream2_put_buffer(&pb, st->codecpar->extradata, extradd);
			//bytestream2_put_buffer(&pb, pkt->data, found_aud - pkt->data);
			//bytestream2_put_buffer(&pb, found_aud_end, buf_end - found_aud_end);
			//av_assert0(new_pkt_size == bytestream2_tell_p(&pb));
			//buf = data;
			//size = new_pkt_size;
		}
        else
        {
            WARNING_EX_LOG(" mpegts failed !!!");
        }


		/// send data net 




        _send_packet(I_frame, payload, payload_size, pts, dts);
        if (data)
        {
            free(data);
            data = NULL;
        }

	}

    static void write_pts(uint8_t* q, int fourbits, int64_t pts)
    {
        int val;

        val = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;
        *q++ = val;
        val = (((pts >> 15) & 0x7fff) << 1) | 1;
        *q++ = val >> 8;
        *q++ = val;
        val = (((pts) & 0x7fff) << 1) | 1;
        *q++ = val >> 8;
        *q++ = val;
    }
	bool cmpegts_encoder::_send_packet(bool I_frame_, uint8* payload, int32 payload_size, int64 pts, int64 dts)
	{
        uint8 buf[TS_PACKET_SIZE] = {0};
        int32 stream_id = 0;
        int32 header_len = 0;
        int32 flags = 0;
        uint8* q = NULL;
        int32 len = 0;
        int32 is_start = 1;
        int32 afc_len = 0;
        int32 stuffing_len = 0;

        bool I_frame = I_frame_;
        while (payload_size > 0) 
        {
            if (I_frame)
            {
                _mpegts_write_sdt();
                _mpegts_write_pat();
                _mpegts_write_pmt();
                I_frame = false;
            }
          //  int64_t pcr = AV_NOPTS_VALUE;
           /* if (ts->mux_rate > 1)
                pcr = get_pcr(ts);
            else if (dts != AV_NOPTS_VALUE)
                pcr = (dts - delay) * 300;*/

            //retransmit_si_info(s, force_pat, force_sdt, force_nit, pcr);
            //force_pat = 0;
            //force_sdt = 0;
            //force_nit = 0;

            //write_pcr = 0;
            //if (ts->mux_rate > 1) {
            //    /* Send PCR packets for all PCR streams if needed */
            //    pcr = get_pcr(ts);
            //    if (pcr >= ts->next_pcr) {
            //        int64_t next_pcr = INT64_MAX;
            //        for (int i = 0; i < s->nb_streams; i++) {
            //            /* Make the current stream the last, because for that we
            //             * can insert the pcr into the payload later */
            //            int st2_index = i < st->index ? i : (i + 1 == s->nb_streams ? st->index : i + 1);
            //            AVStream* st2 = s->streams[st2_index];
            //            MpegTSWriteStream* ts_st2 = st2->priv_data;
            //            if (ts_st2->pcr_period) {
            //                if (pcr - ts_st2->last_pcr >= ts_st2->pcr_period) {
            //                    ts_st2->last_pcr = FFMAX(pcr - ts_st2->pcr_period, ts_st2->last_pcr + ts_st2->pcr_period);
            //                    if (st2 != st) {
            //                        mpegts_insert_pcr_only(s, st2);
            //                        pcr = get_pcr(ts);
            //                    }
            //                    else {
            //                        write_pcr = 1;
            //                    }
            //                }
            //                next_pcr = FFMIN(next_pcr, ts_st2->last_pcr + ts_st2->pcr_period);
            //            }
            //        }
            //        ts->next_pcr = next_pcr;
            //    }
            //    if (dts != AV_NOPTS_VALUE && (dts - pcr / 300) > delay) {
            //        /* pcr insert gets priority over null packet insert */
            //        if (write_pcr)
            //            mpegts_insert_pcr_only(s, st);
            //        else
            //            mpegts_insert_null_packet(s);
            //        /* recalculate write_pcr and possibly retransmit si_info */
            //        continue;
            //    }
            //}
            //else if (ts_st->pcr_period && pcr != AV_NOPTS_VALUE) {
            //    if (pcr - ts_st->last_pcr >= ts_st->pcr_period && is_start) {
            //        ts_st->last_pcr = FFMAX(pcr - ts_st->pcr_period, ts_st->last_pcr + ts_st->pcr_period);
            //        write_pcr = 1;
            //    }
            //}

            /* prepare packet header */
            q = buf;
            *q++ = 0x47;
            int32 pid = 256;
            int32 val = pid >> 8;
            /*if (ts->m2ts_mode && st->codecpar->codec_id == AV_CODEC_ID_AC3)
                val |= 0x20;*/
            if (is_start)
            {
                val |= 0x40;
            }
            *q++ = val;
            *q++ = pid;// ts_st->pid;
            m_cc = m_cc + 1 & 0xf;
            *q++ = 0x10 | m_cc; // payload indicator + CC
            //if (ts_st->discontinuity) {
            //    set_af_flag(buf, 0x80);
            //    q = get_ts_payload_start(buf);
            //    ts_st->discontinuity = 0;
            //}
            //if (!(ts->flags & MPEGTS_FLAG_OMIT_RAI) &&
            //    key && is_start && pts != AV_NOPTS_VALUE &&
            //    !is_dvb_teletext /* adaptation+payload forbidden for teletext (ETSI EN 300 472 V1.3.1 4.1) */) {
            //    // set Random Access for key frames
            //    if (ts_st->pcr_period)
            //        write_pcr = 1;
            //    set_af_flag(buf, 0x40);
            //    q = get_ts_payload_start(buf);
            //}
            //if (write_pcr) {
            //    set_af_flag(buf, 0x10);
            //    q = get_ts_payload_start(buf);
            //    // add 11, pcr references the last byte of program clock reference base
            //    if (dts != AV_NOPTS_VALUE && dts < pcr / 300)
            //        av_log(s, AV_LOG_WARNING, "dts < pcr, TS is invalid\n");
            //    extend_af(buf, write_pcr_bits(q, pcr));
            //    q = get_ts_payload_start(buf);
            //}
             if (is_start) 
            {
                int pes_extension = 0;
                int pes_header_stuffing_bytes = 0;
                int async;
                /* write PES header */
                *q++ = 0x00;
                *q++ = 0x00;
                *q++ = 0x01;
                // TODO@chensong 2024-04-22 解码端识别视频然后进行解码 
                *q++ = stream_id = STREAM_ID_VIDEO_STREAM_0; // get_pes_stream_id(s, st, stream_id, &async);
               // if (async)
                   // pts = dts = AV_NOPTS_VALUE;

                header_len = 0;

                if (stream_id != STREAM_ID_PROGRAM_STREAM_MAP &&
                    stream_id != STREAM_ID_PADDING_STREAM &&
                    stream_id != STREAM_ID_PRIVATE_STREAM_2 &&
                    stream_id != STREAM_ID_ECM_STREAM &&
                    stream_id != STREAM_ID_EMM_STREAM &&
                    stream_id != STREAM_ID_PROGRAM_STREAM_DIRECTORY &&
                    stream_id != STREAM_ID_DSMCC_STREAM &&
                    stream_id != STREAM_ID_TYPE_E_STREAM)  
                {

                    flags = 0;
                    //if (pts != AV_NOPTS_VALUE) 
                    {
                        header_len += 5;
                        flags |= 0x80;
                    }
                    //if (dts != AV_NOPTS_VALUE && pts != AV_NOPTS_VALUE && dts != pts) 
                    {
                        header_len += 5;
                        flags |= 0x40;
                    }
                    //if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
                    //    st->codecpar->codec_id == AV_CODEC_ID_DIRAC) {
                    //    /* set PES_extension_flag */
                    //    pes_extension = 1;
                    //    flags |= 0x01;

                    //    /* One byte for PES2 extension flag +
                    //     * one byte for extension length +
                    //     * one byte for extension id */
                    //    header_len += 3;
                    //}
                    /* for Blu-ray AC3 Audio the PES Extension flag should be as follow
                     * otherwise it will not play sound on blu-ray
                     */
                    //if (ts->m2ts_mode &&
                    //    st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
                    //    st->codecpar->codec_id == AV_CODEC_ID_AC3) {
                    //    /* set PES_extension_flag */
                    //    pes_extension = 1;
                    //    flags |= 0x01;
                    //    header_len += 3;
                    //}
                    /*if (is_dvb_teletext) 
                    {
                        pes_header_stuffing_bytes = 0x24 - header_len;
                        header_len = 0x24;
                    }*/
                    len = payload_size + header_len + 3;
                    /* 3 extra bytes should be added to DVB subtitle payload: 0x20 0x00 at the beginning and trailing 0xff */
                    /*if (is_dvb_subtitle) {
                        len += 3;
                        payload_size++;
                    }*/
                    if (len > 0xffff)
                    {
                        len = 0;
                    }
                   /* if (ts->omit_video_pes_length && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                        len = 0;
                    }*/
                    *q++ = len >> 8;
                    *q++ = len;
                    val = 0x80;
                    /* data alignment indicator is required for subtitle and data streams */
                   /* if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE || st->codecpar->codec_type == AVMEDIA_TYPE_DATA)
                        val |= 0x04;*/
                    *q++ = val;
                    *q++ = flags;
                    *q++ = header_len;
                    //if (pts != AV_NOPTS_VALUE) 
                    {
                        write_pts(q, flags >> 6, pts);
                        q += 5;
                    }
                   // if (dts != AV_NOPTS_VALUE && pts != AV_NOPTS_VALUE && dts != pts) 
                    {
                        write_pts(q, 1, dts);
                        q += 5;
                    }
                    //if (pes_extension && st->codecpar->codec_id == AV_CODEC_ID_DIRAC) {
                    //    flags = 0x01;  /* set PES_extension_flag_2 */
                    //    *q++ = flags;
                    //    *q++ = 0x80 | 0x01; /* marker bit + extension length */
                    //    /* Set the stream ID extension flag bit to 0 and
                    //     * write the extended stream ID. */
                    //    *q++ = 0x00 | 0x60;
                    //}
                    /* For Blu-ray AC3 Audio Setting extended flags */
                    //if (ts->m2ts_mode &&
                    //    pes_extension &&
                    //    st->codecpar->codec_id == AV_CODEC_ID_AC3) {
                    //    flags = 0x01; /* set PES_extension_flag_2 */
                    //    *q++ = flags;
                    //    *q++ = 0x80 | 0x01; /* marker bit + extension length */
                    //    *q++ = 0x00 | 0x71; /* for AC3 Audio (specifically on blue-rays) */
                    //}


                    //if (is_dvb_subtitle) {
                    //    /* First two fields of DVB subtitles PES data:
                    //     * data_identifier: for DVB subtitle streams shall be coded with the value 0x20
                    //     * subtitle_stream_id: for DVB subtitle stream shall be identified by the value 0x00 */
                    //    *q++ = 0x20;
                    //    *q++ = 0x00;
                    //}
                    //if (is_dvb_teletext) {
                    //    memset(q, 0xff, pes_header_stuffing_bytes);
                    //    q += pes_header_stuffing_bytes;
                    //}
                }
                else 
                {
                    len = payload_size;
                    *q++ = len >> 8;
                    *q++ = len;
                }
                is_start = 0;
            }
            /* header size */
            header_len = q - buf;
            /* data len */
            len = TS_PACKET_SIZE - header_len;
            if (len > payload_size)
            {
                len = payload_size;
            }
            stuffing_len = TS_PACKET_SIZE - header_len - len;
            if (stuffing_len > 0) {
                /* add stuffing with AFC */
                if (buf[3] & 0x20) {
                    /* stuffing already present: increase its size */
                    afc_len = buf[4] + 1;
                    memmove(buf + 4 + afc_len + stuffing_len,
                        buf + 4 + afc_len,
                        header_len - (4 + afc_len));
                    buf[4] += stuffing_len;
                    memset(buf + 4 + afc_len, 0xff, stuffing_len);
                }
                else {
                    /* add stuffing */
                    memmove(buf + 4 + stuffing_len, buf + 4, header_len - 4);
                    buf[3] |= 0x20;
                    buf[4] = stuffing_len - 1;
                    if (stuffing_len >= 2) {
                        buf[5] = 0x00;
                        memset(buf + 6, 0xff, stuffing_len - 2);
                    }
                }
            }

            //if (is_dvb_subtitle && payload_size == len) {
            //    memcpy(buf + TS_PACKET_SIZE - len, payload, len - 1);
            //    buf[TS_PACKET_SIZE - 1] = 0xff; /* end_of_PES_data_field_marker: an 8-bit field with fixed contents 0xff for DVB subtitle */
            //}
            //else 
            {
                memcpy(buf + TS_PACKET_SIZE - len, payload, len);
            }

            payload += len;
            payload_size -= len;
           // write_packet(s, buf);
            _send_ts_packet(&buf[0], TS_PACKET_SIZE);
#if 0
            int32 send_len = TS_PACKET_SIZE;
            memcpy(m_send_buffer + m_send_len, &buf[0], send_len);
            m_send_len += send_len;
            if (m_send_len > 1315)
            {
                int32 send_cur = 0;
                while (m_send_len- send_cur > 0)
                {
                    /* ret = sendto(s->udp_fd, buf, size, 0,
                         (struct sockaddr*)&s->dest_addr,
                         s->dest_addr_len);*/
                    NORMAL_EX_LOG("[udp send data len = %u]", send_len);
                    int ret = sendto(m_socket_fd, (const char*)m_send_buffer + send_cur, m_send_len- send_cur, 0, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr));
                    if (ret < 0)
                    {
                        break;
                    }
                    send_cur += ret;
                }
                m_send_len = 0;
            }
#endif 
        }
        //ts_st->prev_payload_key = key;
        return true;
	}


    static av_always_inline av_const uint32_t static_av_bswap32(uint32_t x)
    {
#ifdef _MSC_VER
	        return _byteswap_ulong(x);
#else
		    return (x & 0x000000FF) << 24 | (x & 0x0000FF00) <<  8 |
			               (x & 0x00FF0000) >>  8 | (x & 0xFF000000) >> 24;
#endif
     //   return _byteswap_ulong(x);
    }
    bool cmpegts_encoder::_mpegts_write_section(uint8* buf, int32 len)
    {
        unsigned int crc;
        unsigned char packet[TS_PACKET_SIZE];
        const unsigned char* buf_ptr;
        unsigned char* q;
        int first, b, len1, left;

        crc = static_av_bswap32(av_crc(av_crc_get_table(AV_CRC_32_IEEE),
            -1, buf, len - 4));

        buf[len - 4] = (crc >> 24) & 0xff;
        buf[len - 3] = (crc >> 16) & 0xff;
        buf[len - 2] = (crc >> 8) & 0xff;
        buf[len - 1] = crc & 0xff;

        /* send each packet */
        buf_ptr = buf;
        while (len > 0) 
        {
            first = buf == buf_ptr;
            q = packet;
            *q++ = 0x47;
            int32 pid = 256;
            b = /*s->pid*/pid >> 8;
            if (first)
            {
                b |= 0x40;
            }
            *q++ = b;
            *q++ = pid; // s->pid;
            //s->cc = s->cc + 1 & 0xf;
            m_cc = m_cc + 1 & 0xf;;
            *q++ = 0x10 | m_cc;// s->cc;
            /*if (s->discontinuity) {
                q[-1] |= 0x20;
                *q++ = 1;
                *q++ = 0x80;
                s->discontinuity = 0;
            }*/
            if (first)
            {
                *q++ = 0; /* 0 offset */
            }
            len1 = TS_PACKET_SIZE - (q - packet);
            if (len1 > len)
            {
                len1 = len;
            }
            memcpy(q, buf_ptr, len1);
            q += len1;
            /* add known padding data */
            left = TS_PACKET_SIZE - (q - packet);
            if (left > 0)
            {
                memset(q, 0xff, left);
            }

            //s->write_packet(s, packet);
            _send_ts_packet(&packet[0], TS_PACKET_SIZE);
            buf_ptr += len1;
            len -= len1;
        }
        return true;
    }
    bool cmpegts_encoder::_send_ts_packet(uint8* payload, int32 payload_size)
    {
       // int32 send_len = TS_PACKET_SIZE;
        memcpy(m_send_buffer + m_send_len, payload, payload_size);
        m_send_len += payload_size;
        if (m_send_len > 1315)
        {
           // static FILE* out_ts_file_ptr = ::fopen("./chensong.ts", "wb+");
            int32 send_cur = 0;
            while (m_send_len - send_cur > 0)
            {
                /* ret = sendto(s->udp_fd, buf, size, 0,
                     (struct sockaddr*)&s->dest_addr,
                     s->dest_addr_len);*/
               /* int32 write_ret = ::fwrite((const char*)m_send_buffer + send_cur, 1, m_send_len - send_cur, out_ts_file_ptr);
                fflush(out_ts_file_ptr);
                if (m_send_len - send_cur != write_ret)
                {
                    WARNING_EX_LOG("write file failed (%u)(%u)!!!", m_send_len - send_cur, write_ret);
                }*/
                int ret = ::sendto(m_socket_fd, (const char*)m_send_buffer + send_cur, m_send_len - send_cur, 0, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr));
                if (ret < 0)
                {
                    break;
                }
               // NORMAL_EX_LOG("[udp send data len = %u]", ret);
                send_cur += ret;
            }
            m_send_len = 0;
        }
        return true;
    }
    bool cmpegts_encoder::_mpegts_write_section1(int32 tid, int32 id, int32 version, int32 sec_num, int32 last_sec_num, uint8* buff, int32 len)
    {
        uint8 section[1024], * q;
        uint32 tot_len = 0;
        /* reserved_future_use field must be set to 1 for SDT and NIT */
        uint32 flags = (tid == SDT_TID || tid == NIT_TID) ? 0xf000 : 0xb000;

        tot_len = 3 + 5 + len + 4;
        /* check if not too big */
        if (tot_len > 1024)
        {
            return false;
        }

        q = section;
        *q++ = tid;
        put16(&q, flags | (len + 5 + 4)); /* 5 byte header + 4 byte CRC */
        put16(&q, id);
        *q++ = 0xc1 | (version << 1); /* current_next_indicator = 1 */
        *q++ = sec_num;
        *q++ = last_sec_num;
        memcpy(q, buff, len);

        _mpegts_write_section( section, tot_len);
        return true;
    }
    bool cmpegts_encoder::_mpegts_write_pat()
    {
       /* MpegTSWrite* ts = s->priv_data;
        MpegTSService* service;*/
        uint8_t data[SECTION_LENGTH], * q;
        int i;

        q = data;
       /* if (ts->flags & MPEGTS_FLAG_NIT) 
        {
            put16(&q, 0x0000);
            put16(&q, NIT_PID);
        }*/
        int32 sid = 0;
        int32 pid = 256;
        put16(&q,  sid);
        put16(&q, 0xe000 |  pid);
        /*for (i = 0; i < ts->nb_services; i++) 
        {
            service = ts->services[i];
            put16(&q, service->sid);
            put16(&q, 0xe000 | service->pmt.pid);
        }
        mpegts_write_section1(&ts->pat, PAT_TID, ts->transport_stream_id, ts->tables_version, 0, 0,
            data, q - data);*/
        int32 transport_stream_id = 0;
        int32 tables_version = 0;
        _mpegts_write_section1(PAT_TID, transport_stream_id,  tables_version, 0, 0,
            data, q - data);

        return true;
    }

    bool cmpegts_encoder::_mpegts_write_pmt()
    {
       // MpegTSWrite* ts = s->priv_data;
        uint8_t data[SECTION_LENGTH], * q, * desc_length_ptr, * program_info_length_ptr;
        int val, stream_type, i, err = 0;

        q = data;
        int32 pcr_pid = 0x1fff;
        put16(&q, 0xe000 | pcr_pid);

        program_info_length_ptr = q;
        q += 2; /* patched after */

        /* put program info here */
        //if (ts->m2ts_mode) {
        //    put_registration_descriptor(&q, MKTAG('H', 'D', 'M', 'V'));
        //    *q++ = 0x88;        // descriptor_tag - hdmv_copy_control_descriptor
        //    *q++ = 0x04;        // descriptor_length
        //    put16(&q, 0x0fff);  // CA_System_ID
        //    *q++ = 0xfc;        // private_data_byte
        //    *q++ = 0xfc;        // private_data_byte
        //}

        val = 0xf000 | (q - program_info_length_ptr - 2);
        program_info_length_ptr[0] = val >> 8;
        program_info_length_ptr[1] = val;

       // for (i = 0; i < s->nb_streams; i++) 
        {
           /* AVStream* st = s->streams[i];
            MpegTSWriteStream* ts_st = st->priv_data;
            AVDictionaryEntry* lang = av_dict_get(st->metadata, "language", NULL, 0);*/
            const char default_language[] = "und";
          /*  const char* language = lang && strlen(lang->value) >= 3 ? lang->value : default_language;
            enum AVCodecID codec_id = st->codecpar->codec_id;*/

           /* if (s->nb_programs) {
                int k, found = 0;
                AVProgram* program = service->program;

                for (k = 0; k < program->nb_stream_indexes; k++)
                    if (program->stream_index[k] == i) {
                        found = 1;
                        break;
                    }

                if (!found)
                    continue;
            }*/

            if (q - data > SECTION_LENGTH - 32) 
            {
                err = 1;
                WARNING_EX_LOG("The PMT section cannot fit stream video  and all following streams.\n"
                    "Try reducing the number of languages in the audio streams "
                    "or the total number of streams.\n");
            } 
            else
            {
                stream_type = STREAM_TYPE_VIDEO_H264; // ts->m2ts_mode ? get_m2ts_stream_type(s, st) : get_dvb_stream_type(s, st);
                // TODO@chensong 20240419 H264 有点特殊 
                *q++ = stream_type;
                int32 pid = 256;
                put16(&q, 0xe000 | pid);
                desc_length_ptr = q;
                q += 2; /* patched after */

                /* write optional descriptors here */
                //switch (st->codecpar->codec_type) {
                //case AVMEDIA_TYPE_AUDIO:
                //    if (codec_id == AV_CODEC_ID_AC3)
                //        put_registration_descriptor(&q, MKTAG('A', 'C', '-', '3'));
                //    if (codec_id == AV_CODEC_ID_EAC3)
                //        put_registration_descriptor(&q, MKTAG('E', 'A', 'C', '3'));
                //    if (ts->flags & MPEGTS_FLAG_SYSTEM_B) {
                //        if (codec_id == AV_CODEC_ID_AC3) {
                //            DVBAC3Descriptor* dvb_ac3_desc = ts_st->dvb_ac3_desc;

                //            *q++ = 0x6a; // AC3 descriptor see A038 DVB SI
                //            if (dvb_ac3_desc) {
                //                int len = 1 +
                //                    !!(dvb_ac3_desc->component_type_flag) +
                //                    !!(dvb_ac3_desc->bsid_flag) +
                //                    !!(dvb_ac3_desc->mainid_flag) +
                //                    !!(dvb_ac3_desc->asvc_flag);

                //                *q++ = len;
                //                *q++ = dvb_ac3_desc->component_type_flag << 7 | dvb_ac3_desc->bsid_flag << 6 |
                //                    dvb_ac3_desc->mainid_flag << 5 | dvb_ac3_desc->asvc_flag << 4;

                //                if (dvb_ac3_desc->component_type_flag) *q++ = dvb_ac3_desc->component_type;
                //                if (dvb_ac3_desc->bsid_flag)           *q++ = dvb_ac3_desc->bsid;
                //                if (dvb_ac3_desc->mainid_flag)         *q++ = dvb_ac3_desc->mainid;
                //                if (dvb_ac3_desc->asvc_flag)           *q++ = dvb_ac3_desc->asvc;
                //            }
                //            else {
                //                *q++ = 1; // 1 byte, all flags sets to 0
                //                *q++ = 0; // omit all fields...
                //            }
                //        }
                //        else if (codec_id == AV_CODEC_ID_EAC3) {
                //            *q++ = 0x7a; // EAC3 descriptor see A038 DVB SI
                //            *q++ = 1; // 1 byte, all flags sets to 0
                //            *q++ = 0; // omit all fields...
                //        }
                //    }
                //    if (codec_id == AV_CODEC_ID_S302M)
                //        put_registration_descriptor(&q, MKTAG('B', 'S', 'S', 'D'));
                //    if (codec_id == AV_CODEC_ID_OPUS) {
                //        int ch = st->codecpar->ch_layout.nb_channels;

                //        /* 6 bytes registration descriptor, 4 bytes Opus audio descriptor */
                //        if (q - data > SECTION_LENGTH - 6 - 4) {
                //            err = 1;
                //            break;
                //        }

                //        put_registration_descriptor(&q, MKTAG('O', 'p', 'u', 's'));

                //        *q++ = 0x7f; /* DVB extension descriptor */
                //        *q++ = 2;
                //        *q++ = 0x80;

                //        if (st->codecpar->extradata && st->codecpar->extradata_size >= 19) {
                //            if (st->codecpar->extradata[18] == 0 && ch <= 2) {
                //                /* RTP mapping family */
                //                *q++ = ch;
                //            }
                //            else if (st->codecpar->extradata[18] == 1 && ch <= 8 &&
                //                st->codecpar->extradata_size >= 21 + ch) {
                //                static const uint8_t coupled_stream_counts[9] = {
                //                    1, 0, 1, 1, 2, 2, 2, 3, 3
                //                };
                //                static const uint8_t channel_map_a[8][8] = {
                //                    {0},
                //                    {0, 1},
                //                    {0, 2, 1},
                //                    {0, 1, 2, 3},
                //                    {0, 4, 1, 2, 3},
                //                    {0, 4, 1, 2, 3, 5},
                //                    {0, 4, 1, 2, 3, 5, 6},
                //                    {0, 6, 1, 2, 3, 4, 5, 7},
                //                };
                //                static const uint8_t channel_map_b[8][8] = {
                //                    {0},
                //                    {0, 1},
                //                    {0, 1, 2},
                //                    {0, 1, 2, 3},
                //                    {0, 1, 2, 3, 4},
                //                    {0, 1, 2, 3, 4, 5},
                //                    {0, 1, 2, 3, 4, 5, 6},
                //                    {0, 1, 2, 3, 4, 5, 6, 7},
                //                };
                //                /* Vorbis mapping family */

                //                if (st->codecpar->extradata[19] == ch - coupled_stream_counts[ch] &&
                //                    st->codecpar->extradata[20] == coupled_stream_counts[ch] &&
                //                    memcmp(&st->codecpar->extradata[21], channel_map_a[ch - 1], ch) == 0) {
                //                    *q++ = ch;
                //                }
                //                else if (ch >= 2 && st->codecpar->extradata[19] == ch &&
                //                    st->codecpar->extradata[20] == 0 &&
                //                    memcmp(&st->codecpar->extradata[21], channel_map_b[ch - 1], ch) == 0) {
                //                    *q++ = ch | 0x80;
                //                }
                //                else {
                //                    /* Unsupported, could write an extended descriptor here */
                //                    av_log(s, AV_LOG_ERROR, "Unsupported Opus Vorbis-style channel mapping");
                //                    *q++ = 0xff;
                //                }
                //            }
                //            else {
                //                /* Unsupported */
                //                av_log(s, AV_LOG_ERROR, "Unsupported Opus channel mapping for family %d", st->codecpar->extradata[18]);
                //                *q++ = 0xff;
                //            }
                //        }
                //        else if (ch <= 2) {
                //            /* Assume RTP mapping family */
                //            *q++ = ch;
                //        }
                //        else {
                //            /* Unsupported */
                //            av_log(s, AV_LOG_ERROR, "Unsupported Opus channel mapping");
                //            *q++ = 0xff;
                //        }
                //    }

                //    if (language != default_language ||
                //        st->disposition & (AV_DISPOSITION_CLEAN_EFFECTS |
                //            AV_DISPOSITION_HEARING_IMPAIRED |
                //            AV_DISPOSITION_VISUAL_IMPAIRED)) {
                //        const char* p, * next;
                //        uint8_t* len_ptr;

                //        *q++ = ISO_639_LANGUAGE_DESCRIPTOR;
                //        len_ptr = q++;
                //        *len_ptr = 0;

                //        for (p = next = language; next && *len_ptr < 255 / 4 * 4; p = next + 1) {
                //            if (q - data > SECTION_LENGTH - 4) {
                //                err = 1;
                //                break;
                //            }
                //            next = strchr(p, ',');
                //            if (strlen(p) != 3 && (!next || next != p + 3))
                //                continue; /* not a 3-letter code */

                //            *q++ = *p++;
                //            *q++ = *p++;
                //            *q++ = *p++;

                //            if (st->disposition & AV_DISPOSITION_CLEAN_EFFECTS)
                //                *q++ = 0x01;
                //            else if (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED)
                //                *q++ = 0x02;
                //            else if (st->disposition & AV_DISPOSITION_VISUAL_IMPAIRED)
                //                *q++ = 0x03;
                //            else
                //                *q++ = 0; /* undefined type */

                //            *len_ptr += 4;
                //        }

                //        if (*len_ptr == 0)
                //            q -= 2; /* no language codes were written */
                //    }
                //    break;
                //case AVMEDIA_TYPE_SUBTITLE:
                //    if (codec_id == AV_CODEC_ID_DVB_SUBTITLE) {
                //        uint8_t* len_ptr;
                //        int extradata_copied = 0;

                //        *q++ = 0x59; /* subtitling_descriptor */
                //        len_ptr = q++;

                //        while (strlen(language) >= 3) {
                //            if (sizeof(data) - (q - data) < 8) { /* 8 bytes per DVB subtitle substream data */
                //                err = 1;
                //                break;
                //            }
                //            *q++ = *language++;
                //            *q++ = *language++;
                //            *q++ = *language++;
                //            /* Skip comma */
                //            if (*language != '\0')
                //                language++;

                //            if (st->codecpar->extradata_size - extradata_copied >= 5) {
                //                *q++ = st->codecpar->extradata[extradata_copied + 4]; /* subtitling_type */
                //                memcpy(q, st->codecpar->extradata + extradata_copied, 4); /* composition_page_id and ancillary_page_id */
                //                extradata_copied += 5;
                //                q += 4;
                //            }
                //            else {
                //                /* subtitling_type:
                //                 * 0x10 - normal with no monitor aspect ratio criticality
                //                 * 0x20 - for the hard of hearing with no monitor aspect ratio criticality */
                //                *q++ = (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED) ? 0x20 : 0x10;
                //                if ((st->codecpar->extradata_size == 4) && (extradata_copied == 0)) {
                //                    /* support of old 4-byte extradata format */
                //                    memcpy(q, st->codecpar->extradata, 4); /* composition_page_id and ancillary_page_id */
                //                    extradata_copied += 4;
                //                    q += 4;
                //                }
                //                else {
                //                    put16(&q, 1); /* composition_page_id */
                //                    put16(&q, 1); /* ancillary_page_id */
                //                }
                //            }
                //        }

                //        *len_ptr = q - len_ptr - 1;
                //    }
                //    else if (codec_id == AV_CODEC_ID_DVB_TELETEXT) {
                //        uint8_t* len_ptr = NULL;
                //        int extradata_copied = 0;

                //        /* The descriptor tag. teletext_descriptor */
                //        *q++ = 0x56;
                //        len_ptr = q++;

                //        while (strlen(language) >= 3 && q - data < sizeof(data) - 6) {
                //            *q++ = *language++;
                //            *q++ = *language++;
                //            *q++ = *language++;
                //            /* Skip comma */
                //            if (*language != '\0')
                //                language++;

                //            if (st->codecpar->extradata_size - 1 > extradata_copied) {
                //                memcpy(q, st->codecpar->extradata + extradata_copied, 2);
                //                extradata_copied += 2;
                //                q += 2;
                //            }
                //            else {
                //                /* The Teletext descriptor:
                //                 * teletext_type: This 5-bit field indicates the type of Teletext page indicated. (0x01 Initial Teletext page)
                //                 * teletext_magazine_number: This is a 3-bit field which identifies the magazine number.
                //                 * teletext_page_number: This is an 8-bit field giving two 4-bit hex digits identifying the page number. */
                //                *q++ = 0x08;
                //                *q++ = 0x00;
                //            }
                //        }

                //        *len_ptr = q - len_ptr - 1;
                //    }
                //    else if (codec_id == AV_CODEC_ID_ARIB_CAPTION) {
                //        if (put_arib_caption_descriptor(s, &q, st->codecpar) < 0)
                //            break;
                //    }
                //    break;
                //case AVMEDIA_TYPE_VIDEO:
                //    if (stream_type == STREAM_TYPE_VIDEO_DIRAC) {
                //        put_registration_descriptor(&q, MKTAG('d', 'r', 'a', 'c'));
                //    }
                //    else if (stream_type == STREAM_TYPE_VIDEO_VC1) {
                //        put_registration_descriptor(&q, MKTAG('V', 'C', '-', '1'));
                //    }
                //    else if (stream_type == STREAM_TYPE_VIDEO_HEVC && s->strict_std_compliance <= FF_COMPLIANCE_NORMAL) {
                //        put_registration_descriptor(&q, MKTAG('H', 'E', 'V', 'C'));
                //    }
                //    else if (stream_type == STREAM_TYPE_VIDEO_CAVS || stream_type == STREAM_TYPE_VIDEO_AVS2 ||
                //        stream_type == STREAM_TYPE_VIDEO_AVS3) {
                //        put_registration_descriptor(&q, MKTAG('A', 'V', 'S', 'V'));
                //    }
                //    break;
                //case AVMEDIA_TYPE_DATA:
                //    if (codec_id == AV_CODEC_ID_SMPTE_KLV) {
                //        put_registration_descriptor(&q, MKTAG('K', 'L', 'V', 'A'));
                //    }
                //    else if (codec_id == AV_CODEC_ID_SMPTE_2038) {
                //        put_registration_descriptor(&q, MKTAG('V', 'A', 'N', 'C'));
                //    }
                //    else if (codec_id == AV_CODEC_ID_TIMED_ID3) {
                //        const char* tag = "ID3 ";
                //        *q++ = METADATA_DESCRIPTOR;
                //        *q++ = 13;
                //        put16(&q, 0xffff);    /* metadata application format */
                //        putbuf(&q, tag, strlen(tag));
                //        *q++ = 0xff;        /* metadata format */
                //        putbuf(&q, tag, strlen(tag));
                //        *q++ = 0;            /* metadata service ID */
                //        *q++ = 0xF;          /* metadata_locator_record_flag|MPEG_carriage_flags|reserved */
                //    }
                //    break;
                //}

                val = 0xf000 | (q - desc_length_ptr - 2);
                desc_length_ptr[0] = val >> 8;
                desc_length_ptr[1] = val;
            }
        }

      /*  if (err)
            av_log(s, AV_LOG_ERROR,
                "The PMT section cannot fit stream %d and all following streams.\n"
                "Try reducing the number of languages in the audio streams "
                "or the total number of streams.\n", i);

        mpegts_write_section1(&service->pmt, PMT_TID, service->sid, ts->tables_version, 0, 0,
            data, q - data);*/
        int32 sid = 0;
        int32 tables_version = 0;
        _mpegts_write_section1(PMT_TID, sid, tables_version, 0, 0, data, q -data);
        return true;
    }
    bool cmpegts_encoder::_mpegts_write_nit()
    {
        //MpegTSWrite* ts = s->priv_data;
        //uint8_t data[SECTION_LENGTH], * q, * desc_len_ptr, * loop_len_ptr;

        //q = data;

        ////network_descriptors_length
        //put16(&q, 0xf000 | (ts->provider_name[0] + 2));

        ////network_name_descriptor
        //*q++ = 0x40;
        //putbuf(&q, ts->provider_name, ts->provider_name[0] + 1);

        ////transport_stream_loop_length
        //loop_len_ptr = q;
        //q += 2;

        //put16(&q, ts->transport_stream_id);
        //put16(&q, ts->original_network_id);

        ////transport_descriptors_length
        //desc_len_ptr = q;
        //q += 2;

        ////service_list_descriptor
        //*q++ = 0x41;
        //*q++ = 3 * ts->nb_services;
        //for (int i = 0; i < ts->nb_services; i++) {
        //    put16(&q, ts->services[i]->sid);
        //    *q++ = ts->service_type;
        //}

        ////calculate lengths
        //put16(&desc_len_ptr, 0xf000 | q - (desc_len_ptr + 2));
        //put16(&loop_len_ptr, 0xf000 | q - (loop_len_ptr + 2));

       /* mpegts_write_section1(&ts->nit, NIT_TID, ts->original_network_id, ts->tables_version, 0, 0,
            data, q - data);*/
        return true;
    }
    bool cmpegts_encoder::_mpegts_write_sdt()
    {
        uint8_t data[SECTION_LENGTH], * q, * desc_list_len_ptr, * desc_len_ptr;
        int i, running_status, free_ca_mode, val;

        q = data;
        put16(&q, 0/*ts->original_network_id*/);
        *q++ = 0xff;
        {
            int32 sid = 0;
            put16(&q, sid/*service->sid*/);
            *q++ = 0xfc | 0x00; /* currently no EIT info */
            desc_list_len_ptr = q;
            q += 2;
            running_status = 4; /* running */
            free_ca_mode = 0;

            /* write only one descriptor for the service name and provider */
            *q++ = 0x48;
            desc_len_ptr = q;
            q++;
            *q++ = MPEGTS_SERVICE_TYPE_MPEG2_DIGITAL_HDTV;// ts->service_type;
            putbuf(&q, (const uint8*)DEFAULT_PROVIDER_NAME, strlen((const char *)DEFAULT_PROVIDER_NAME));
            putbuf(&q, (const uint8*)DEFAULT_SERVICE_NAME, strlen((const char*)DEFAULT_SERVICE_NAME));
            desc_len_ptr[0] = q - desc_len_ptr - 1;

            /* fill descriptor length */
            val = (running_status << 13) | (free_ca_mode << 12) |
                (q - desc_list_len_ptr - 2);
            desc_list_len_ptr[0] = val >> 8;
            desc_list_len_ptr[1] = val;
        }
        int32 transport_stream_id = 0;
        int32 tables_version = 0;
        _mpegts_write_section1(  SDT_TID, transport_stream_id, tables_version, 0, 0,
            data, q - data);
        //MpegTSWrite* ts = s->priv_data;
        //MpegTSService* service;
        //uint8_t data[SECTION_LENGTH], * q, * desc_list_len_ptr, * desc_len_ptr;
        //int i, running_status, free_ca_mode, val;

        //q = data;
        //put16(&q, ts->original_network_id);
        //*q++ = 0xff;
        //for (i = 0; i < ts->nb_services; i++) {
        //    service = ts->services[i];
        //    put16(&q, service->sid);
        //    *q++ = 0xfc | 0x00; /* currently no EIT info */
        //    desc_list_len_ptr = q;
        //    q += 2;
        //    running_status = 4; /* running */
        //    free_ca_mode = 0;

        //    /* write only one descriptor for the service name and provider */
        //    *q++ = 0x48;
        //    desc_len_ptr = q;
        //    q++;
        //    *q++ = ts->service_type;
        //    putbuf(&q, service->provider_name, service->provider_name[0] + 1);
        //    putbuf(&q, service->name, service->name[0] + 1);
        //    desc_len_ptr[0] = q - desc_len_ptr - 1;

        //    /* fill descriptor length */
        //    val = (running_status << 13) | (free_ca_mode << 12) |
        //        (q - desc_list_len_ptr - 2);
        //    desc_list_len_ptr[0] = val >> 8;
        //    desc_list_len_ptr[1] = val;
        //}
        /*mpegts_write_section1(&ts->sdt, SDT_TID, ts->transport_stream_id, ts->tables_version, 0, 0,
            data, q - data);*/
        return true;
    }


    void cmpegts_encoder::_pthread_work()
    {
        cmpegts_page* mpegts_ptr = NULL;
        while (!m_stoped)
        {
            {
                std::unique_lock<std::mutex> lock(m_mpgets_page_lock);
                m_condition.wait(lock, [this]() {return m_mpegts_page_list.size() > 0 || m_stoped; });
            }
           // _handler_check_log_file();
            while (!m_mpegts_page_list.empty())
            {
                 {
                    std::lock_guard<std::mutex> lock{ m_mpgets_page_lock };
                    mpegts_ptr = &m_mpegts_page_list.front();
                    m_mpegts_page_list.pop_front();
                }

                if (!mpegts_ptr)
                {
                    continue;
                } 


                push_packet(mpegts_ptr->I_frame, mpegts_ptr->data, mpegts_ptr->size, mpegts_ptr->pts, mpegts_ptr->dts);
                if (mpegts_ptr->data)
                {
                    delete[] mpegts_ptr->data;
                    mpegts_ptr->data = NULL;
                }
                mpegts_ptr = NULL;
               // _handler_log_item(log_item_ptr);

               /* if (log_item_ptr->buf)
                {
                    delete[] log_item_ptr->buf;
                }

                delete log_item_ptr;*/

            }
        }
    }
}
