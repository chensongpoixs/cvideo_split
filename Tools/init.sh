#!/bin/bash


自己写一个shell脚本
将写好的脚本（.sh文件）放到目录 /etc/profile.d/ 下，系统启动后就会自动执行该目录下的所有shell脚本。

```
cd /etc/profile.d/
```

复制
添加脚本 srs.sh

```
#!/bin/sh

cd /usr/local/srs2
nohup ./objs/srs -c conf/z.conf>./log.txt &


```