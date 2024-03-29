#include "device.h"
 
#include "pugixml.hpp"
#include "gb28181_header_maker.h"
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netinet/udp.h>


#ifdef WIN32
/* #include <syslog.h> */
#include <winsock2.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#endif
#include <string>
#include <sstream>
#include <thread>
#include <tuple> 
#include "clog.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace chen {


static int SN_MAX = 99999999;
static int sn;

static int get_sn() {
	if (sn >= SN_MAX) {
		sn = 0;
	}
	sn++;
	return sn;
}

void Device::push_rtp_stream() {
    is_pushing = true;

    int status = this->bind();
    if (status != 0) {
       printf("device bind socket address failed: {%u}", status);
        return ;
    }

	char ps_header[PS_HDR_LEN];

	char ps_system_header[SYS_HDR_LEN];

	char ps_map_header[PSM_HDR_LEN];

	char pes_header[PES_HDR_LEN];

	char rtp_header[RTP_HDR_LEN];    

	int time_base = 90000;
	int fps = 24;
	int send_packet_interval = 1000 / fps;

	int interval = time_base / fps;
	long pts = 0;

	char frame[1024 * 128];

	int single_packet_max_length = 1400;

	char rtp_packet[RTP_HDR_LEN+1400];

	// int ssrc = 0xffffffff;
	int rtp_seq = 0;

    // Nalu *nalu = new Nalu();
	// nalu->packet = (char *)malloc(1024*128);
	// nalu->length = 1024 * 128;

    while (is_pushing) {
        for (auto i = 0; i < nalu_vector.size(); i++) {
            auto nalu = nalu_vector.at(i);

            NaluType  type = nalu->type;
            int length = nalu->length;
            char * packet = nalu->packet;

            int index = 0;
            if (NALU_TYPE_IDR == type) {
                gb28181_make_ps_header(ps_header, pts);

                memcpy(frame,ps_header,PS_HDR_LEN);
                index += PS_HDR_LEN;

                gb28181_make_sys_header(ps_system_header, 0x3f);

                memcpy(frame+ index, ps_system_header, SYS_HDR_LEN);
                index += SYS_HDR_LEN;

                gb28181_make_psm_header(ps_map_header);

                memcpy(frame + index, ps_map_header, PSM_HDR_LEN);
                index += PSM_HDR_LEN;

            } else {
                gb28181_make_ps_header(ps_header, pts);

                memcpy(frame, ps_header, PS_HDR_LEN);
                index += PS_HDR_LEN;
            }

            //封装pes
            gb28181_make_pes_header(pes_header, 0xe0, length, pts, pts);

            memcpy(frame+index, pes_header, PES_HDR_LEN);
            index += PES_HDR_LEN;

            memcpy(frame + index, packet, length);
            index += length;

            //组包rtp

            int rtp_packet_count = ((index - 1) / single_packet_max_length) + 1;

            for (int i = 0; i < rtp_packet_count; i++) {

                gb28181_make_rtp_header(rtp_header, rtp_seq, pts, atoi(ssrc.c_str()), i == (rtp_packet_count - 1));

                int writed_count = single_packet_max_length;

                if ((i + 1)*single_packet_max_length > index) {
                    writed_count = index - (i* single_packet_max_length);
                }
                //添加包长字节
                int rtp_start_index=0;

                unsigned short rtp_packet_length = RTP_HDR_LEN + writed_count;
                if (rtp_protocol == "TCP/RTP/AVP") {
                    unsigned char packt_length_ary[2];
                    packt_length_ary[0] = (rtp_packet_length >> 8) & 0xff;
                    packt_length_ary[1] = rtp_packet_length & 0xff;
                    memcpy(rtp_packet, packt_length_ary, 2);
                    rtp_start_index = 2;
                }

                memcpy(rtp_packet+ rtp_start_index, rtp_header, RTP_HDR_LEN);
                memcpy(rtp_packet+ +rtp_start_index + RTP_HDR_LEN, frame+ (i* single_packet_max_length), writed_count);
                rtp_seq++;

                if (is_pushing) {
                    send_network_packet(rtp_packet, rtp_start_index + rtp_packet_length);
                }
                else {
                    if (nalu != nullptr) {
                        delete nalu;
                        nalu = nullptr;
                    }
                    return;
                }
            }

            pts += interval;

            std::this_thread::sleep_for(std::chrono::milliseconds(send_packet_interval));
        }
    }

    is_pushing = false;
}

void Device::start() {
    printf("sip init begin.");

    sip_context = eXosip_malloc();

    if (OSIP_SUCCESS != eXosip_init(sip_context)) {
        //spdlog::error("sip init failed.");
        ERROR_EX_LOG("sip init failed.");
        return;
    }

    if (OSIP_SUCCESS != eXosip_listen_addr(sip_context, IPPROTO_UDP, nullptr, local_port, AF_INET, 0)) {
        WARNING_EX_LOG("sip port bind failed.");
        eXosip_quit(sip_context);
        sip_context = nullptr;
        return;
    }

    // run
    is_running = true;

    std::ostringstream from_uri;
    std::ostringstream contact;
    std::ostringstream proxy_uri;

    // local ip & port
    eXosip_guess_localip(sip_context, AF_INET, (char*)(local_ip.c_str()), local_ip.length());
    NORMAL_EX_LOG("local ip is {%s}", local_ip.c_str());

    from_uri << "sip:" << channel_sip_id << "@" << local_ip << ":" << local_port;
    contact << "sip:" << device_sip_id << "@" << local_ip << ":" << local_port;
    proxy_uri << "sip:" << server_sip_id << "@" << server_ip << ":" << server_port;

    from_sip = from_uri.str();
    to_sip = proxy_uri.str();

    NORMAL_EX_LOG("from uri is {%s}", from_sip.c_str());
    NORMAL_EX_LOG("contact is {%s}", contact.str().c_str());
    NORMAL_EX_LOG("proxy_uri is {%s}", to_sip.c_str());

    // clear auth
    eXosip_clear_authentication_info(sip_context);

    osip_message_t * register_message = nullptr;
    int register_id = eXosip_register_build_initial_register(sip_context, from_sip.c_str(), 
                    to_sip.c_str(), 
                    contact.str().c_str(), 3600, &register_message);
    if (nullptr == register_message) {
        ERROR_EX_LOG("eXosip_register_build_initial_register failed");
        return;
    }

    eXosip_lock(sip_context);
	eXosip_register_send_register(sip_context, register_id, register_message);
	eXosip_unlock(sip_context);

    std::thread heartbeat_task_thread(&Device::heartbeat_task, this);
    heartbeat_task_thread.detach();

    this->process_request();
}

void Device::process_request() {
    while (is_running) {
        auto evt = std::shared_ptr<eXosip_event_t>(
            eXosip_event_wait(sip_context, 0, 100),
            eXosip_event_free);

        eXosip_lock(sip_context);
        eXosip_automatic_action(sip_context);
        eXosip_unlock(sip_context);

        if (evt == nullptr) {
            continue;
        }

        switch (evt->type)
        {
        case eXosip_event_type::EXOSIP_REGISTRATION_SUCCESS: {
            NORMAL_EX_LOG("got REGISTRATION_SUCCESS");
            is_register = true;
            break;
        }
        case eXosip_event_type::EXOSIP_REGISTRATION_FAILURE: {
            NORMAL_EX_LOG("got REGISTRATION_FAILURE");
            if (evt->response == nullptr) {
                WARNING_EX_LOG("register 401 has no response !!!");
                break;
            }

            if (401 == evt->response->status_code) {
                osip_www_authenticate_t * www_authenticate_header;

                osip_message_get_www_authenticate(evt->response, 0, &www_authenticate_header);

                if (eXosip_add_authentication_info(sip_context, channel_sip_id.c_str(), username.c_str(), password.c_str(), 
                                    "MD5", www_authenticate_header->realm)) {
                    WARNING_EX_LOG("register add auth failed");
                    break;
                };
            };
            break;
        }
        case eXosip_event_type::EXOSIP_MESSAGE_NEW: {
            NORMAL_EX_LOG("got MESSAGE_NEW");

            if (MSG_IS_MESSAGE(evt->request)) {
                osip_body_t * body = nullptr;
                osip_message_get_body(evt->request, 0, &body);
                if (body != nullptr) {
                    NORMAL_EX_LOG("new message request: \n{%s}", body->body);
                }

                this->send_response_ok(evt);

                auto cmd_sn = this->get_cmd(body->body);
                std::string cmd = std::get<0>(cmd_sn);
                std::string sn = std::get<1>(cmd_sn);
                NORMAL_EX_LOG("got new cmd: {%s}", cmd.c_str());
                if ("Catalog" == cmd) {
                    this->process_catalog_query(sn);
                } else if ("DeviceStatus" == cmd) {
                    this->process_devicestatus_query(sn);
                } else if ("DeviceInfo" == cmd) {
                    this->process_deviceinfo_query(sn);
                } else if ("DeviceControl" == cmd) {
                    this->process_devicecontrol_query(sn);
                } else {
                    WARNING_EX_LOG("unhandled cmd: {%s}", cmd.c_str());
                }
            } else if (MSG_IS_BYE(evt->request)) {
                WARNING_EX_LOG("got BYE message");
                this->send_response_ok(evt);
                break;
            }
            break;
        }
        case eXosip_event_type::EXOSIP_CALL_INVITE: {
            NORMAL_EX_LOG("got CALL_INVITE");

            auto sdp_msg = eXosip_get_remote_sdp(sip_context, evt->did);
            if (!sdp_msg) {
                WARNING_EX_LOG("eXosip_get_remote_sdp failed");
                break;
            }

            auto connection = eXosip_get_video_connection(sdp_msg);
            if (!connection) {
                WARNING_EX_LOG("eXosip_get_video_connection failed");
                break;                
            }

            rtp_ip = connection->c_addr;

            auto video_sdp = eXosip_get_video_media(sdp_msg);
            if (!video_sdp) {
                WARNING_EX_LOG("eXosip_get_video_media failed");
                break;                  
            }

            rtp_port = atoi(video_sdp->m_port);

            NORMAL_EX_LOG("rtp server: {%s}:{%u}", rtp_ip.c_str(), rtp_port);

            rtp_protocol = video_sdp->m_proto;

            NORMAL_EX_LOG("rtp protocol: {%s}", rtp_protocol.c_str());

            osip_body_t *sdp_body = NULL;
			osip_message_get_body(evt->request, 0, &sdp_body);
            if (nullptr == sdp_body) {
                WARNING_EX_LOG("osip_message_get_body failed");
                break; 
            }

            std::string body = sdp_body->body;
            auto y_sdp_first_index = body.find("y=");
            auto y_sdp = body.substr(y_sdp_first_index);
            auto y_sdp_last_index = y_sdp.find("\r\n");
            ssrc = y_sdp.substr(2, y_sdp_last_index-1);
            NORMAL_EX_LOG("ssrc: {%u}", ssrc.c_str());

            std::stringstream ss;
            ss << "v=0\r\n";
            ss << "o=" << device_sip_id << " 0 0 IN IP4 " << local_ip << "\r\n";
            ss << "s=Play\r\n";
            ss << "c=IN IP4 " << local_ip << "\r\n";
            ss << "t=0 0\r\n";
            if (rtp_protocol == "TCP/RTP/AVP") {
                ss << "m=video " << local_port << " TCP/RTP/AVP 96\r\n";
            }
            else {
                ss << "m=video " << local_port << " RTP/AVP 96\r\n";
            }
            ss << "a=sendonly\r\n";
            ss << "a=rtpmap:96 PS/90000\r\n";
            ss << "y=" << ssrc << "\r\n";
            std::string sdp_output_str  = ss.str();

            size_t size = sdp_output_str.size();

            osip_message_t * message = evt->request;
            int status = eXosip_call_build_answer(sip_context, evt->tid, 200, &message);

            if (status != 0) {
                WARNING_EX_LOG("call invite build answer failed");
                break;
            }
            
            osip_message_set_content_type(message, "APPLICATION/SDP");
            osip_message_set_body(message, sdp_output_str.c_str(), sdp_output_str.size());

            eXosip_call_send_answer(sip_context, evt->tid, 200, message);

            NORMAL_EX_LOG("reply call invite: \n{%s}", sdp_output_str.c_str());
            break;
        }
        case eXosip_event_type::EXOSIP_CALL_ACK: {
            NORMAL_EX_LOG("got CALL_ACK: begin pushing rtp stream...");
            if (is_pushing) {
                NORMAL_EX_LOG("already pushing rtp stream");
            } else {
                std::thread t(&Device::push_rtp_stream, this);
                t.detach();
            }
            break;
        }
        case eXosip_event_type::EXOSIP_CALL_CLOSED: {
            WARNING_EX_LOG("got CALL_CLOSED: stop pushing rtp stream...");

            break;
        }
        case eXosip_event_type::EXOSIP_MESSAGE_ANSWERED: {
            NORMAL_EX_LOG("got MESSAGE_ANSWERED: unhandled");
            break;
        }
        
        default: {
            WARNING_EX_LOG("unhandled sip evt type: {%u}", evt->type);
            break;
        }
        }
    }
}

void Device::process_catalog_query(std::string sn) {
    std::stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n";
    ss << "<Response>\r\n";
    ss << "<CmdType>Catalog</CmdType>\r\n";
    ss << "<SN>" << sn << "</SN>\r\n";
    ss << "<DeviceID>" << device_sip_id << "</DeviceID>\r\n";
    ss << "<SumNum>" << 1 << "</SumNum>\r\n";
    ss << "<DeviceList Num=\"" << 1 << "\">\r\n";
    ss << "<Item>\r\n";
    ss << "<DeviceID>" << channel_sip_id << "</DeviceID>\r\n";
    ss << "<Manufacturer>" << manufacture << "</Manufacturer>\r\n";
    ss << "<Status>ON</Status>\r\n";
    ss << "<Name>IPC</Name>\r\n";
    ss << "<ParentID>" << server_sip_id << "</ParentID>\r\n";
    ss << "</Item>\r\n";
    ss << "</DeviceList>\r\n";
    ss << "</Response>\r\n";
    NORMAL_EX_LOG("catalog response: \n{}", ss.str());
    auto request = create_msg();
    if (request != NULL) {
        osip_message_set_content_type(request, "Application/MANSCDP+xml");
        osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
        send_request(request);
    }
}

void Device::process_devicestatus_query(std::string sn) {
    std::stringstream ss;

    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char curtime[72] = {0};
    sprintf(curtime, "%d-%d-%dT%02d:%02d:%02d", (timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1),
                        timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);    
    
    ss << "<?xml version=\"1.0\"?>\r\n";
    ss << "<Response>\r\n";
    ss << "<CmdType>DeviceStatus</CmdType>\r\n";
    ss << "<SN>" << get_sn() << "</SN>\r\n";
    ss << "<DeviceID>" << device_sip_id << "</DeviceID>\r\n";
    ss << "<Result>OK</Result>\r\n";
    ss << "<Online>ONLINE</Online>\r\n";
    ss << "<Status>OK</Status>\r\n";
    ss << "<DeviceTime>" << curtime << "</DeviceTime>\r\n";
    ss << "<Alarmstatus Num=\"0\">\r\n";
    ss << "</Alarmstatus>\r\n";
    ss << "<Encode>ON</Encode>\r\n";
    ss << "<Record>OFF</Record>\r\n";
    ss << "</Response>\r\n";

    NORMAL_EX_LOG("devicestatus response: \n{%s}", ss.str().c_str());
    auto request = create_msg();
    if (request != NULL) {
        osip_message_set_content_type(request, "Application/MANSCDP+xml");
        osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
        send_request(request);
    }
}

void Device::process_deviceinfo_query(std::string sn) {
    std::stringstream ss;

    ss << "<?xml version=\"1.0\"?>\r\n";
    ss <<    "<Response>\r\n";
    ss <<    "<CmdType>DeviceInfo</CmdType>\r\n";
    ss <<    "<SN>" << get_sn() << "</SN>\r\n";
    ss <<    "<DeviceID>" << device_sip_id << "</DeviceID>\r\n";
    ss <<    "<Result>OK</Result>\r\n";
    ss <<    "<DeviceType>simulate client</DeviceType>\r\n";
    ss <<    "<Manufacturer>ZHD</Manufacturer>\r\n";
    ss <<    "<Model>28181</Model>\r\n";
    ss <<    "<Firmware>fireware</Firmware>\r\n";
    ss <<    "<MaxCamera>1</MaxCamera>\r\n";
    ss <<    "<MaxAlarm>0</MaxAlarm>\r\n";
    ss <<    "</Response>\r\n";

    NORMAL_EX_LOG("deviceinfo response: \n{}", ss.str());
    auto request = create_msg();
    if (request != NULL) {
        osip_message_set_content_type(request, "Application/MANSCDP+xml");
        osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
        send_request(request);
    }
}

void Device::process_devicecontrol_query(std::string sn) {

}

void Device::heartbeat_task() {
	while (true) {
        if (is_register) {
            std::stringstream ss;
            ss << "<?xml version=\"1.0\"?>\r\n";
            ss << "<Notify>\r\n";
            ss << "<CmdType>Keepalive</CmdType>\r\n";
            ss << "<SN>" << get_sn() << "</SN>\r\n";
            ss << "<DeviceID>" << device_sip_id << "</DeviceID>\r\n";
            ss << "<Status>OK</Status>\r\n";
            ss << "</Notify>\r\n";

            osip_message_t* request = create_msg();
            if (request != NULL) {
                osip_message_set_content_type(request, "Application/MANSCDP+xml");
                osip_message_set_body(request, ss.str().c_str(), strlen(ss.str().c_str()));
                send_request(request);
                NORMAL_EX_LOG("sent heartbeat");
            }
        }

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
}

osip_message_t * Device::create_msg() {

    osip_message_t * request = nullptr;
    auto status = eXosip_message_build_request(sip_context, &request, "MESSAGE", to_sip.c_str(), from_sip.c_str(), nullptr);
    if (OSIP_SUCCESS != status) {
        WARNING_EX_LOG("build request failed: {%u}", status);
    }

    return request;
}

void Device::send_request(osip_message_t * request) {
    eXosip_lock(sip_context);
    eXosip_message_send_request(sip_context, request);
    eXosip_unlock(sip_context);
}

void Device::send_response(std::shared_ptr<eXosip_event_t> evt, osip_message_t * msg) {
    eXosip_lock(sip_context);
    eXosip_message_send_answer(sip_context, evt->tid, 200, msg);
    eXosip_unlock(sip_context);
}

void Device::send_response_ok(std::shared_ptr<eXosip_event_t> evt) {
    auto msg = evt->request;
    eXosip_message_build_answer(sip_context, evt->tid, 200, &msg);
    send_response(evt, msg);
}

std::tuple<std::string, std::string> Device::get_cmd(const char * body) {
    pugi::xml_document document;

    if (!document.load_string(body)) {
        WARNING_EX_LOG("cannot parse the xml");
        return std::make_tuple("", "");
    }

    pugi::xml_node root_node = document.first_child();

    if (!root_node) {
        WARNING_EX_LOG("cannot get root node of xml");
        return std::make_tuple("", "");
    }

    std::string root_name = root_node.name();
    if ("Query" != root_name) {
        WARNING_EX_LOG("invalid query xml with root: {%s}", root_name.c_str());
        return std::make_tuple("", "");
    }

    auto cmd_node = root_node.child("CmdType");

    if (!cmd_node) {
        WARNING_EX_LOG("cannot get the cmd type");
        return std::make_tuple("", "");
    }

    auto sn_node = root_node.child("SN");

    if (!sn_node) {
        WARNING_EX_LOG("cannot get the SN");
        return std::make_tuple("", "");
    }

    std::string cmd = cmd_node.child_value();
    std::string sn = sn_node.child_value();

    return std::make_tuple(cmd, sn);
}

}
