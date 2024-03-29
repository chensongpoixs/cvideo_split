project(common_sip_client_device)
cmake_minimum_required(VERSION 2.8)
MESSAGE(STATUS "#########################")
ADD_DEFINITIONS(-std=c++11   -fpermissive )
file(GLOB ALL_CXX_SRCS
        ./*.cpp  
		./*.cc
        ./*.h
		./*.c
		
		./c-ares/*.cpp  
		./c-ares/*.cc
        ./c-ares/*.h
		./c-ares/*.c
		
		
		./eXosip2/*.cpp  
		./eXosip2/*.cc
        ./eXosip2/*.h
		./eXosip2/*.c
         
		 
		./osip2/*.cpp  
		./osip2/*.cc
        ./osip2/*.h
		./osip2/*.c
		
		
		./osipparser2/*.cpp  
		./osipparser2/*.cc
        ./osipparser2/*.h
		./osipparser2/*.c
		
		
		./osipparser2/headers/*.cpp  
		./osipparser2/headers/*.cc
        ./osipparser2/headers/*.h
		./osipparser2/headers/*.c
        )

add_library(${PROJECT_NAME} ${ALL_CXX_SRCS})
target_link_libraries(${PROJECT_NAME} )