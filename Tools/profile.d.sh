Linux设置开机启动的三种方法

方法一 添加命令

编辑文件 /etc/rc.local

vim /ect/rc.local

复制

/ect/rc.local和/ect/rc.d/rc.local是软链接关系

chmod +x /ect/rc.d/rc.local
复制
方法二 添加脚本
自己写一个shell脚本
将写好的脚本（.sh文件）放到目录 /etc/profile.d/ 下，系统启动后就会自动执行该目录下的所有shell脚本。

cd /etc/profile.d/
复制
添加脚本 srs.sh

#!/bin/sh

cd /usr/local/srs2
nohup ./objs/srs -c conf/z.conf>./log.txt &
复制
方法二 添加服务
添加文件
新建/etc/init.d/srs.sh 文件

#!/bin/sh
# chkconfig: 2345 85 15
# description:auto_run

#程序根位置
MY_ROOT=/usr/local/srs2/

#运行程序位置
MY_PATH="${MY_ROOT}objs/srs" 

#LOG位置
LOG_PATH="$MY_ROOT"log.txt

#开始方法
start() {
    cd $MY_ROOT
    nohup $MY_PATH -c conf/z.conf>$LOG_PATH &
    echo "$MY_PATH start success."
}

#结束方法
stop() {
    kill -9 `ps -ef|grep $MY_PATH|grep -v grep|grep -v stop|awk '{print $2}'`
    echo "$MY_PATH stop success."
}

case "$1" in
start)
    start
    ;;
stop)
    stop
    ;;
restart)
    stop
    start
    ;;
*)
    echo "Userage: $0 {start|stop|restart}"
    exit 1
esac
复制
添加执行权限
给sh文件和jar可执行权限

chmod +x /etc/init.d/srs.sh
复制
设置开机启动
首先，添加为系统服务

chkconfig --add srs.sh
复制
开机自启动

chkconfig srs.sh on
复制
查看

chkconfig --list
复制
启动

service srs.sh start
复制
停用

service srs.sh stop
复制
查看启动情况

lsof -i:1935
复制
本文参与 腾讯云自媒体分享计划，分享自作者个人站点/博客。