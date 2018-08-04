#!/bin/bash

# 移动配置文件
cp -f ./shadowsock-backup.json /etc/shadowsocks.json

# 当前内核
uname -r

# 安装锐速
wget -N --no-check-certificate https://github.com/91yun/serverspeeder/raw/master/serverspeeder.sh && bash serverspeeder.sh

# 锐速优化
cp -f ./serverspeeder-config /serverspeeder/etc/config
/serverspeeder/bin/serverSpeeder.sh restart

# TCP（非BBR）优化，并设置ulimit
echo "* soft nofile 51200
* hard nofile 51200" >> /etc/security/limits.conf
ulimit -n 51200

# 加载hybla算法
/sbin/modprobe tcp_hybla

# TCP配置优化
echo "#TCP配置优化(不然你自己根本不知道你在干什么)
fs.file-max = 51200
#提高整个系统的文件限制
net.core.rmem_max = 67108864
net.core.wmem_max = 67108864
net.core.netdev_max_backlog = 250000
net.core.somaxconn = 4096
net.ipv4.tcp_syncookies = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_tw_recycle = 0
net.ipv4.tcp_fin_timeout = 30
net.ipv4.tcp_keepalive_time = 1200
net.ipv4.ip_local_port_range = 10000 65000
net.ipv4.tcp_max_syn_backlog = 8192
net.ipv4.tcp_max_tw_buckets = 5000
net.ipv4.tcp_fastopen = 3
net.ipv4.tcp_mem = 25600 51200 102400
net.ipv4.tcp_rmem = 4096 87380 67108864
net.ipv4.tcp_wmem = 4096 65536 67108864
net.ipv4.tcp_mtu_probing = 1
net.ipv4.tcp_congestion_control = hybla
#END OF LINE" >> /etc/sysctl.conf
sysctl -p

/etc/init.d/shadowsocks restart

# 开机自启ssr和加速器
echo "/etc/init.d/shadowsocks restart
/serverspeeder/bin/serverSpeeder.sh restart
touch" >> /etc/rc.local

# 设置iptables
if [ ! -d "/etc/iptables-rules" ]; then
    mkdir /etc/iptables-rules
fi
/etc/init.d/iptables start
iptables-restore < ./rules.raw

# 添加manager命令到系统
echo "export PATH=\"/root/vps-server:\$PATH\"" >> /etc/profile
source /etc/profile

touch scanlog.log
touch server.log
touch vpscronlog.log

# 修改文件权限
chmod a+wx /root/vps-server/manager
chmod a+w /root/vps-server/scanlog.log
chmod a+w /root/vps-server/server.log
chmod a+w /root/vps-server/shadowsock-backup.json
chmod a+w /root/vps-server/vpscron
chmod a+w /root/vps-server/vpscronlog.log 

# 添加定时任务
crontab /root/vps-server/vpscron

# 更换系统时间
cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime

/etc/init.d/shadowsocks restart
/serverspeeder/bin/serverSpeeder.sh restart
