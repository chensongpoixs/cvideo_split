#project(crobot)
project(cvideo_split)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
MESSAGE(STATUS "#########################")
ADD_DEFINITIONS(-std=c++11   -fpermissive -fvisibility=hidden)

# RTC

# /client/rtc

#SET(FFMPEG_INCLUDE_DIR /home)
#set(CMAKE_CXX_FLAGS         "-std=c++11  -g  -Wall ")  #注意调试的话,此行代码一定要添加!-fno-rtti
  


#LINK_DIRECTORIES(
#        /usr/local/lib
#        /usr/local/lib64
#        /usr/lib/x86_64-linux-gnu
#        ${PROJECT_SOURCE_DIR}/linux/lib
#
#)

#INCLUDE_DIRECTORIES(
#        ${RTC_INC_DIR}/client/rtc)
#LINK_DIRECTORIES(
#        #../ccloud_game_rtc/client
#
#        ${RTC_INC_DIR}/linux/lib
#        ${WEBRTC_DIR}/lib/
#        /usr/local/cuda/lib64/stubs
#)

file(GLOB ALL_CXX_SRCS
         ./*.h
		 ./*.cpp 
		 ./*.hpp 
		 ./*.cc 
        )

#add_library(${PROJECT_NAME}  SHARED   ${ALL_CXX_SRCS}   ) #SHARED  STATIC
add_executable(${PROJECT_NAME} ${ALL_CXX_SRCS})
add_dependencies(${PROJECT_NAME} common)
add_dependencies(${PROJECT_NAME} clib)
add_dependencies(${PROJECT_NAME} common_ffmpeg)
add_dependencies(${PROJECT_NAME} common_web)
add_dependencies(${PROJECT_NAME} common_websocket)
add_dependencies(${PROJECT_NAME} common_sip)

set(ALL_LIB  	pthread
 
	#	X11
	#		vdpau
		libcommon.a 
		libcommon_web.a 
		libcommon_ffmpeg.a 
		libcommon_websocket.a 
		libresolv.so
		libcommon_sip.a
		libclib.a 
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
		#
		libfreetype.a
		libharfbuzz.a

		#libharfbuzz-subset.a
		libxml2.a
		libfontconfig.a
		#				libfribidi.a
		libdl.so
		#libdl.a
		 libboost_atomic.a
		libboost_chrono.a
		libboost_container.a
		libboost_context.a
		libboost_coroutine.a
		libboost_date_time.a
		libboost_exception.a
		libboost_fiber.a
		libboost_filesystem.a
		libboost_graph.a
		libboost_iostreams.a
		libboost_locale.a
		libboost_log.a
		libboost_log_setup.a
		libboost_math_c99.a
		libboost_math_c99f.a
		libboost_math_c99l.a
		libboost_math_tr1.a
		libboost_math_tr1f.a
		libboost_math_tr1l.a
		libboost_prg_exec_monitor.a
		libboost_program_options.a
		libboost_random.a
		libboost_regex.a
		libboost_serialization.a
		libboost_signals.a
		libboost_stacktrace_addr2line.a
		libboost_stacktrace_backtrace.a
		libboost_stacktrace_basic.a
		libboost_stacktrace_noop.a
		libboost_system.a
		libboost_test_exec_monitor.a
		libboost_thread.a
		libboost_timer.a
		libboost_type_erasure.a
		libboost_unit_test_framework.a
		libboost_wave.a
		libboost_wserialization.a
		libjsoncpp.a
		libprotobuf.a
        )


target_link_libraries(${PROJECT_NAME} ${ALL_LIB})
