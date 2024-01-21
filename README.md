#  视频拼接

 
ffmepg 支持非常丰富的推流命令，简单介绍一下使用 ffmpeg 和 ffplay 在局域网使用 udp协议推拉流。

UDP 单播推拉流

# udp 单播推流

```
ffmpeg.exe -re -stream_loop -1 -i .\JFLA.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://192.168.22.83:10189"
```

# udp 单播拉流

```
ffplay.exe -f h264 "udp://192.168.22.83:10189" -fflags nobuffer -nofind_stream_info
```
 
UDP 组播推拉流

udp组播地址

组播地址范围 224.0.0.0 – 239.255.255.255

224.0.0.0–224.0.0.255 本地保留，给知名协议使用，ttl=1。其中224.0.0.1是本网所有主机接收，224.0.0.2是本网所有路由器接收。

# udp 组播推流

// 169.254.119.31

```
ffmpeg.exe -re -stream_loop -1 -i .\input.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://239.0.0.1:54546?buffer_size=0&localaddr=192.168.22.119" 


# cmd
ffmpeg.exe -re -stream_loop -1 -i .\input.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://239.0.0.1:54546?buffer_size=0&localaddr=169.254.119.31" 
```

# udp 组播拉流

```
ffplay.exe -f h264 "udp://239.0.0.1:54546" -fflags nobuffer -nofind_stream_info
```
 
UDP 广播推拉流

# udp 广播推流

```
ffmpeg.exe -re -stream_loop -1 -i .\JFLA.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://192.168.22.255:10189"
```

# udp拉流

```
ffplay.exe -f h264 "udp://192.168.22.119:10189" -fflags nobuffer -nofind_stream_info
```

 