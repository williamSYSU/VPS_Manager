# VPS Manager
针对ShadowsocksR服务器端软件对用户进行管理，主要包括分配端口、设置密码、设置使用时长等操作。

## How to use
* cJSON.c, cJOSN.h
> 处理JSON文件用到的第三方库
* manager.c, manager
> 自动管理程序的源码以及其可执行文件
* vpscron
> 定时任务列表，包含：
> 1. 8-22点之间每隔十五分钟记录当前连接的端口以及对于IP，输出保存在vpscronlog.log文件中
> 2. 每隔四个小时扫描一次已过期端口，输出保存在scanlog.log文件中
> 3. 每隔五分钟重启一次SSR
* currentuser.sh
> 用于查看当前连接到服务器所使用的端口以及其对应IP

## Compile manager.c
```bash
gcc cJSON.c manager.c -o manager -lm -w
```

## Command guide
```bash
manager -add {port} {month}		添加一个用户，设置使用时间
manager -batch {number}	                批量添加用户，输入添加个数
manager -activate {port} {month}	激活某个端口，并设置使用时间
manager -prolong {port} {month}		延长某个端口的使用时间
manager -delete {port}			删除某个端口
manager -get {port}			得到某个端口的密码
manager -change {port}                  改变某个端口的密码
manager -info {port}			获得某个端口的信息
manager -back				备份配置文件到当前目录
manager -refresh			更新ShadowsockR软件
manager -scan				扫描定时器，删除过期端口
manager -count				获得总用户数
manager -recount			重新统计总用户数
manager -countfalse			统计无效端口数
manager -deletefalse		        删除无效端口数
manager -help				显示程序帮助信息
```

## Other references
* [科学上网教程（一）——VPS上搭建SSR|默](https://jasper-1024.github.io/2016/06/26/VPS%E7%A7%91%E5%AD%A6%E4%B8%8A%E7%BD%91%E6%95%99%E7%A8%8B%E7%B3%BB%E5%88%97/)
* [科学上网教程（二）——VPS上优化加速|默](https://jasper-1024.github.io/2016/06/27/VPS%E7%A7%91%E5%AD%A6%E4%B8%8A%E7%BD%91%E6%95%99%E7%A8%8B%E7%B3%BB%E5%88%972/)
* [科学上网教程（三）——VPS上安全加固|默](http://jasper-1024.github.io/2017/04/08/VPS%E7%A7%91%E5%AD%A6%E4%B8%8A%E7%BD%91%E6%95%99%E7%A8%8B3/)
* [教程：一键检测VPS是Openvz还是KVM还是Xen](http://jasper-1024.github.io/2017/04/08/VPS%E7%A7%91%E5%AD%A6%E4%B8%8A%E7%BD%91%E6%95%99%E7%A8%8B3/)
* [教程：CentOS更换内核，提供锐速可用的内核下载](https://www.91yun.org/archives/795)
* [Shadowsocks查看服务器上用户连接数的方法](https://doub.io/ss-jc33/)
* [ShadowsocksR MudbJSON模式多用户一键脚本 支持流量限制等](https://doub.io/ss-jc60/)
* [Digital Ocean VPS各节点测速](http://speedtest-sfo2.digitalocean.com/)
* [Vultr VPS 各节点测速](http://www.vultrvps.com/test-server)
* [多个地点ping服务器网站测速](http://ping.chinaz.com/)
