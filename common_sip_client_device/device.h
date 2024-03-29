#ifndef DEVICE_INCLUDE
#define DEVICE_INCLUDE

#include <string>
#include <tuple>
#include <memory>
#include "eXosip2/eXosip.h"
#include "load_h264.h"

namespace chen {


 
class Device {
public:
    Device() {}

    Device(std::string server_sip_id, std::string server_ip, int server_port,
        std::string device_sip_id,  std::string channel_sip_id, std::string username, std::string password,
            int local_port,
        std::string manufacture,
        std::string filepath):
            server_sip_id(server_sip_id), 
            
            server_ip(server_ip),
            server_port(server_port),
            device_sip_id(device_sip_id),
        channel_sip_id(channel_sip_id),
            username(username),
            password(password),
            local_port(local_port),
            manufacture(manufacture),
            filepath(filepath) {
        sip_context = nullptr;
        is_running = false;
        is_register = false;
        local_ip = std::string(128, '0');

        load(filepath.c_str());
    }

    ~Device(){}

    void start();

    void stop();

    void process_request();

    void process_catalog_query(std::string sn);

    void process_deviceinfo_query(std::string sn);

    void process_devicestatus_query(std::string sn);

    void process_devicecontrol_query(std::string sn);

    void heartbeat_task();

    void send_request(osip_message_t * request);

    void send_response(std::shared_ptr<eXosip_event_t> evt, osip_message_t * msg);

    osip_message_t * create_msg();

    void send_response_ok(std::shared_ptr<eXosip_event_t> evt);

    std::tuple<std::string, std::string> get_cmd(const char * body);

    void push_rtp_stream();

public:
    std::string server_sip_id;
    std::string server_ip;
    int server_port;
    std::string device_sip_id;
    std::string channel_sip_id;
    std::string username;
    std::string password;
    std::string local_ip;
    int local_port;

    std::string manufacture;
    std::string rtp_ip;
    int rtp_port;
    std::string rtp_protocol;

    std::string filepath;

private:
    eXosip_t* sip_context;
    bool is_running;
    bool is_register;
    bool is_pushing;

    std::string from_sip;
    std::string to_sip;
    std::string ssrc;

    int sockfd;
    int bind();
    void send_network_packet(const char * data, int length);
};
}
#endif