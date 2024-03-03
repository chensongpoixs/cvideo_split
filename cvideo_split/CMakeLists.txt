#project(crobot)
project(cvideo_split)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
MESSAGE(STATUS "#########################")

# RTC
SET(RTC_INC_DIR /home/chensong/Work/ccloud_game_rtc)
# /client/rtc

set(CMAKE_CXX_FLAGS         "-std=c++11  -g  ")  #注意调试的话,此行代码一定要添加!-fno-rtti
add_definitions(-D WEBRTC_POSIX) # -fPIC
add_definitions(-D WEBRTC_LINUX)
#


#set(CMAKE_CXX_FLAGS         "-std=c++11 -g    -fno-rtti  ")  #注意调试的话,此行代码一定要添加!
#add_definitions(-D WEBRTC_POSIX) # -fPIC
#add_definitions(-D WEBRTC_LINUX)


SET(WEBRTC_DIR /home/chensong/dep/webrtc)

INCLUDE_DIRECTORIES(
        /usr/local/include
        /usr/include 
        /usr/local/cuda/include
        ${WEBRTC_DIR}/include
        ${WEBRTC_DIR}/include/third_party/libyuv/include
        ${WEBRTC_DIR}/include/third_party/abseil-cpp
        ${WEBRTC_DIR}/include/third_party/jsoncpp/source/include
        ${WEBRTC_DIR}/include/third_party/jsoncpp/generated
        #${WEBRTC_DIR}/include/third_party/googletest/src/googletest/include
        ${WEBRTC_DIR}/gen
      #  ${RTC_INC_DIR}/client/rtc
        ../ccloud_game_rtc/client/rtc
        ../ccloud_game_rtc/client/sdp
        ../ccloud_game_rtc/client
       # ../ccloud_game_rtc/client/glad/include
        ../Robot
        ../ccloud_game_rtc/client/nvcodec 
		../ccloud_game_rtc/client/vaapi
        
)


#LINK_DIRECTORIES(
#        /usr/local/lib
#        /usr/local/lib64
#        /usr/lib/x86_64-linux-gnu
#        ${PROJECT_SOURCE_DIR}/linux/lib
#
#)

#INCLUDE_DIRECTORIES(
#        ${RTC_INC_DIR}/client/rtc)
LINK_DIRECTORIES(
        #../ccloud_game_rtc/client

        ${RTC_INC_DIR}/linux/lib
        ${WEBRTC_DIR}/lib/
        /usr/local/cuda/lib64/stubs
)

file(GLOB ALL_CXX_SRCS
        ../clib/*.cpp
        ../clib/*.h
        ../common/*.cpp
        ../common/*.h
        ../common/cclient_msg/*.cc
        ../common/cclient_msg/*.h
        ./*.cpp  ./*.cc
        ../ccloud_game_rtc/client/*.cc
        ../ccloud_game_rtc/client/*.h
        # ../ccloud_game_rtc/client/rtc/cgl_egl_common.c
        ../ccloud_game_rtc/client/*.cpp
        ../ccloud_game_rtc/client/*.hpp
        ../ccloud_game_rtc/client/rtc/*.cpp
        ../ccloud_game_rtc/client/rtc/*.h
        ../ccloud_game_rtc/client/rtc/*.cc
        
        ../ccloud_game_rtc/client/rtc/*.hpp
        ../ccloud_game_rtc/client/sdp/*.cpp
        ../ccloud_game_rtc/client/sdp/*.h
        ../ccloud_game_rtc/client/sdp/*.cc
        ../ccloud_game_rtc/client/sdp/*.hpp
       # ../ccloud_game_rtc/client/glad/*.c
        ../ccloud_game_rtc/client/nvcodec/*.cpp 
        )

#add_library(${PROJECT_NAME}  SHARED   ${ALL_CXX_SRCS}   ) #SHARED  STATIC
add_executable(${PROJECT_NAME} ${ALL_CXX_SRCS})
#add_dependencies(${PROJECT_NAME} common)
#add_dependencies(${PROJECT_NAME} clib)

set(ALL_LIB  	pthread
 
        pthread
        libavcodec.a
		libavdevice.a   
		libavfilter.a  
		libavformat.a   
		libavutil.a     
		libpostproc.a   
		libx264.a
		libswresample.a
		libswscale.a
		libavfilter.a
		libavcodec.a
		libdl.so
		#libdl.a
        )


target_link_libraries(${PROJECT_NAME} ${ALL_LIB})
