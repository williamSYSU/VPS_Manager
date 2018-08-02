# How to build server
--
This file describe how to build vps server for yourself. Including ShadowsocksR setting and Linux setting.

## Select VPS Server
* Vultr
> https://my.vultr.com/
* Digital Ocean
> https://www.digitalocean.com/
* OS Select
> CentOS 6 x64

## SpeedTest in VPS Server
* Vultr
> [Vultr VPS 各节点测速](http://www.vultrvps.com/test-server)
* Digital Ocean
> [Digital Ocean VPS各节点测速](http://speedtest-sfo2.digitalocean.com/)
* Other
> [多个地点ping服务器网站测速](http://ping.chinaz.com/)

## Software requirement
* XShell
* XFTP
* Notepad++

## 1. Install SSR
```bash
wget --no-check-certificate	 https://raw.githubusercontent.com/teddysun/shadowsocks_install/master/shadowsocksR.sh
chmod +x shadowsocksR.sh
./shadowsocksR.sh 2>&1 | tee shadowsocksR.log
```

## 2. Check if kernel is kvm
* Only kvm support serverspeeder
```bash
wget -N --no-check-certificate https://raw.githubusercontent.com/91yun/code/master/vm_check.sh && bash vm_check.sh
```

## 3. Replace the kernel that  fits serverspeeder
* CentOS 6: replace as 2.6.32-504.3.3.el6.x86_64
```bash
rpm -ivh http://soft.91yun.org/ISO/Linux/CentOS/kernel/kernel-firmware-2.6.32-504.3.3.el6.noarch.rpm
rpm -ivh http://soft.91yun.org/ISO/Linux/CentOS/kernel/kernel-2.6.32-504.3.3.el6.x86_64.rpm --force
```
* CentOS 7: replace as 3.10.0-229.1.2.el7.x86_64
```bash
rpm -ivh http://soft.91yun.org/ISO/Linux/CentOS/kernel/kernel-3.10.0-229.1.2.el7.x86_64.rpm --force
```

## 4. Check if the kernel is installed
* Check if the output contains the version number of the targeted kernel
```bash
rpm -qa | grep kernel
```

## 5. Reboot and Check current kernel
```bash
reboot

uname -r
```

## 6. Install ServerSpeeder and configure
### Install ServerSpeeder
```bash
wget -N --no-check-certificate https://github.com/91yun/serverspeeder/raw/master/serverspeeder.sh && bash serverspeeder.sh
```
* Or uninstall ServerSpeeder
```bash
chattr -i /serverspeeder/etc/apx* && /serverspeeder/bin/serverSpeeder.sh uninstall -f
```
### Configure
* Edit '*/serverspeeder/etc/config*':
```buildoutcfg
rsc=”1″ #RSC 网卡驱动模式
advinacc=”1″ #流量方向加速
maxmode=”1″ #最大传输模式

initialCwndWan="64″
l2wQLimit="1024 4096″
w2lQLimit="1024 4096″

halfCwndMinSRtt="500″
halfCwndLossRateShift="3″
```
### Restart ServerSpeeder
```bash
/serverspeeder/bin/serverSpeeder.sh restart
```

## 7. TCP (not BBR) Optimization
### Increase the number of TCP connections
* Edit (add at last) '*/etc/security/limits.conf*':
```text
* soft nofile 51200
* hard nofile 51200
```
* Set ulimit:
```bash
ulimit -n 51200
```
### Set hybla algorithm
* Check exiting algorithms
```bash
sysctl net.ipv4.tcp_available_congestion_control
```
* Load hybla
```bash
/sbin/modprobe tcp_hybla
```
### Configure optimization
* Edit (add at last) '*/etc/sysctl.conf*':
```buildoutcfg
#TCP配置优化(不然你自己根本不知道你在干什么)
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
#END OF LINE
```
### Apply changes
```bash
sysctl -p
```
### Restart SSR
```bash
/etc/init.d/shadowsocks restart
```

## 8. Test VPS performance (Optional)
```bash
wget -N --no-check-certificate https://github.com/teddysun/across/blob/master/bench.sh && wget -qO- bench.sh | bash
```

## 9. Set SSR and ServerSpeeder Start up
* Edit (add before 'touch') '*/etc/rc.local*'
```bash
/etc/init.d/shadowsocks restart
/serverspeeder/bin/serverSpeeder.sh restart
```

## 10. Disable iptables (Necessary, or not support multi-user)
* Disable permanently and temporarily
```bash
chkconfig iptables off
/etc/init.d/iptables stop
```

## 11. Add 'manager' command
### Add command
* Edit (add at last) '*/etc/profile*':
```bash
export PATH="/root/vps-server:$PATH"
```
* Apply changes
```bash
source /etc/profile
```

## 12. Add Crontab
```bash
crontab /root/vps-server/vpscron
```

## 13. Change system time
```bash
cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
```

## Other restart command
* Restart SSR
```bash
/etc/init.d/shadowsocks restart
```
* Restart SeverSpeeder
```bash
/serverspeeder/bin/serverSpeeder.sh restart
```