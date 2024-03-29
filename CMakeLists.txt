project(Server)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
MESSAGE(STATUS "#########################")
SET(CMAKE_BUILD_TYPE Release)
#ADD_DEFINITIONS(-std=c++11   -fpermissive )
#ADD_DEFINITIONS(-std=c++11   -fpermissive )
ADD_DEFINITIONS(-DHAVE_NETDB_H 
		-DHAVE_NETINET_IN_H
		-DHAVE_SYS_IOCTL_H
		-D__POCC__
		-DHAVE_LIMITS_H
		-DHAVE_FCNTL_H
		-DHAVE_FCNTL_O_NONBLOCK
		-DHAVE_STRUCT_TIMEVAL=0
		-DHAVE_PTHREAD
		-DHAVE_SEMAPHORE_H
		-DHAVE_ERRNO_H
		#-DHAVE_RECV
		#-DHAVE_SEND
		#-DRECV_TYPE_RETV
		#-DHAVE_SEND
		-DHAVE_SYS_SOCKET_H
		#-DHAVE_CONFIG_H   
		#-DHAVE_SETSOCKOPT_SO_NONBLOCK
		)

SET(EXTRA_INC_DIR /home/pc/Work/dep)
SET(FFMPEG_INCLUDE_DIR /home/pc/Work/ffmpeg)

#SET(EXTRA_INCLUDE_DIR /home/chensong/Work/dep)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(
	/usr/include
        /usr/local/include
        ${EXTRA_INC_DIR}/boost/include
        ${EXTRA_INC_DIR}/thrift/include
        ${EXTRA_INC_DIR}/protobuf/include

#        ${EXTRA_INC_DIR}/include
#        ${EXTRA_INC_DIR}/include/mysql
        ${PROJECT_SOURCE_DIR}/clib
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/common_web
        ${PROJECT_SOURCE_DIR}/common_websocket
        ${PROJECT_SOURCE_DIR}/common_ffmpeg
        ${PROJECT_SOURCE_DIR}/common/http_message
        ${PROJECT_SOURCE_DIR}/common/cmsg
        ${PROJECT_SOURCE_DIR}/common/cclient_msg
		${PROJECT_SOURCE_DIR}/common_sip
		${PROJECT_SOURCE_DIR}/common_sip/c-ares
		${PROJECT_SOURCE_DIR}/common_sip/eXosip2
		${PROJECT_SOURCE_DIR}/common_sip/osip2
		${PROJECT_SOURCE_DIR}/common_sip/osipparser2
		${PROJECT_SOURCE_DIR}/common_sip/osipparser2/headers
		
        ${PROJECT_SOURCE_DIR}/cloud_thrift
		
		#${EXTRA_INC_DIR}/libavcodec/  
		#${EXTRA_INC_DIR}/libavdevice/ 
		#${EXTRA_INC_DIR}/libavfilter/ 
		#${EXTRA_INC_DIR}/libavformat/ 
		#${EXTRA_INC_DIR}/libavutil/
		
		${EXTRA_INC_DIR}/boost/include 
		${EXTRA_INC_DIR}/protobuf/include 
		${EXTRA_INC_DIR}/jsoncpp/include 
		
		${FFMPEG_INCLUDE_DIR}
		#${FFMPEG_INCLUDE_DIR}/libavcodec/
		#${FFMPEG_INCLUDE_DIR}/libavdevice/
		#${FFMPEG_INCLUDE_DIR}/libavfilter/
		#${FFMPEG_INCLUDE_DIR}/libavformat/
		#${FFMPEG_INCLUDE_DIR}/libavutil/
        #${PROJECT_SOURCE_DIR}/Robot
)




SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

LINK_DIRECTORIES(
        /usr/local/lib
	/usr/lib
	/usr/lib64
        /usr/local/lib64
        /usr/lib64/mysql/
        /usr/lib/x86_64-linux-gnu
	${FFMPEG_INCLUDE_DIR}/libavcodec/
	                ${FFMPEG_INCLUDE_DIR}/libavdevice/
			                ${FFMPEG_INCLUDE_DIR}/libavfilter/
					                ${FFMPEG_INCLUDE_DIR}/libavformat/
							                ${FFMPEG_INCLUDE_DIR}/libavutil/
									${FFMPEG_INCLUDE_DIR}/libpostproc
									${FFMPEG_INCLUDE_DIR}/libswscale
									${FFMPEG_INCLUDE_DIR}/libswresample
        ${PROJECT_SOURCE_DIR}/linux/lib
        ${EXTRA_INC_DIR}/boost/lib
        ${EXTRA_INC_DIR}/thrift/lib
        ${EXTRA_INC_DIR}/protobuf/lib
        ${EXTRA_INC_DIR}/jsoncpp/lib
)

#add_subdirectory(ccloud_game_rtc/client)
add_subdirectory(common_sip) 
add_subdirectory(clib)
add_subdirectory(common) 
add_subdirectory(common_web) 
add_subdirectory(common_ffmpeg) 
add_subdirectory(common_websocket) 
add_subdirectory(cvideo_split) 
add_subdirectory(decoder) 
