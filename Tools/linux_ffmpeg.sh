


# avfilter drawtext 模块编译  --enable-libfreetype   --enable-libfontconfig  --enable-libfribidi --enable-libharfbuzz  
# --enable-static   --enable-gpl   --enable-cross-compile
# harfbuzz-6.0.0
# 第三方依赖
#c@pc-ProLiant-DL560-Gen10:~/Work/ffmpeg$ cd ../lib/
#pc@pc-ProLiant-DL560-Gen10:~/Work/lib$ ls
#1.7.2.tar.gz  boost_1_66_0         freetype-2.13.1         fribidi-1.0.3   jsoncpp-1.7.2  libass-0.17.1.tar.gz  libfontconfig-5.1.0.tar.gz 
# libxml2-2.9.11  v1.0.13.tar.gz  v2.12.0.tar.gz  v2.9.11.tar.gz
#8.3.0.tar.gz  boost_1_66_0.tar.gz  freetype-2.13.1.tar.gz  harfbuzz-8.3.0  libass-0.17.1  libfontconfig-5.1.0   libxml2-2.12.0       
#       protobuf-3.7.0  v1.0.3.tar.gz   v2.12.1.tar.gz  v3.7.0.tar.gz
#
#

#################################################################################################################
 ./configure     --enable-version3  --enable-cuda --enable-cuvid --enable-nvdec --enable-nvenc   --enable-avfilter \
 --enable-libx264    --enable-libfreetype   --enable-libfontconfig  --enable-libfribidi --enable-libharfbuzz  \
 --enable-static   --enable-gpl   --enable-cross-compile  --extra-cflags="-I/home/pc/Work/nv-codec-headers -I/usr/local/include" \
 --extra-ldflags="-L/usr/lib -L/usr/local/lib" --pkg-config="pkg-config --static"
