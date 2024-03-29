 
#include "device.h"
#ifdef _MSC_VER 
/* #include <syslog.h> */
 #include <winsock2.h>
 #include <Windows.h>
#include <ws2tcpip.h> // Windows
#include <WS2tcpip.h>

#include <ws2tcpip.h> // Windows
#include <Ws2tcpip.h> // 包含头文件
#pragma comment(lib, "Ws2_32.lib") // 链接库
#else 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <sys/types.h>
#include "clog.h"
#include <stdio.h>
#include <stdlib.h>
//#include "udpDefine.h"
//#include <windows.h>
//#include <Windows.h>
// 或者
//#include <arpa/inet.h> // UNIX/Linux



// 使用 inet_pton 的代码

namespace chen { 
#ifdef _MSC_VER 
    int inet_pton(int af, const char* src, void* dst)
    {
        struct sockaddr_storage ss;
        int size = sizeof(ss);
        char src_copy[INET6_ADDRSTRLEN + 1];

        ZeroMemory(&ss, sizeof(ss));
        /* stupid non-const API */
        strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
        src_copy[INET6_ADDRSTRLEN] = 0;

        if (WSAStringToAddress((LPWSTR)src_copy, af, NULL, (struct sockaddr*)&ss, &size) == 0) {
            switch (af) {
            case AF_INET:
                *(struct in_addr*)dst = ((struct sockaddr_in*)&ss)->sin_addr;
                return 1;
            case AF_INET6:
                *(struct in6_addr*)dst = ((struct sockaddr_in6*)&ss)->sin6_addr;
                return 1;
            }
        }
        return 0;
    }
#endif // 
    int Device::bind() 
    {

        if (rtp_protocol == "TCP/RTP/AVP") {
            sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        } else {
            sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        }

        sockaddr_in bind_address;
        bind_address.sin_family = AF_INET;
        bind_address.sin_port = htons(local_port);
        bind_address.sin_addr.s_addr = inet_addr(local_ip.c_str());

        auto bind_status = ::bind(sockfd, (struct sockaddr *)&bind_address, sizeof(bind_address));
        if (bind_status) {
            WARNING_EX_LOG("socket bind failed: {%u}", bind_status);
            return bind_status;
        }

        sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(rtp_port);
        inet_pton(AF_INET, rtp_ip.c_str(), &server_address.sin_addr);

        auto connect_status = ::connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
        if ( connect_status < 0) {
           WARNING_EX_LOG("socket connect failed: {%u}", connect_status);
            return connect_status;
        }

    return 0;
}

void Device::send_network_packet(const char * data, int length) 
{
    if (rtp_protocol == "TCP/RTP/AVP") {
        ::send(sockfd, data, length, 0);
    } else {
        sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(rtp_port);
        inet_pton(AF_INET, rtp_ip.c_str(), &server_address.sin_addr);
        ::sendto(sockfd, data, length, 0, (struct sockaddr *)&server_address, sizeof(server_address));
    }
}
 }