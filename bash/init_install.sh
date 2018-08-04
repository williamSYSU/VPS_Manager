#!/bin/bash

# 初始化
if [ ! -d "./vps-server" ]; then
	mkdir ./vps-server
fi
mv ./init_config.sh ./vps-server/

# 安装SSR
wget --no-check-certificate	 https://raw.githubusercontent.com/teddysun/shadowsocks_install/master/shadowsocksR.sh
chmod +x shadowsocksR.sh
./shadowsocksR.sh 2>&1 | tee shadowsocksR.log

# 检测是Openvz还是kvm
wget -N --no-check-certificate https://raw.githubusercontent.com/91yun/code/master/vm_check.sh && bash vm_check.sh

# 更换适合91云加速的内核：
rpm -ivh http://soft.91yun.org/ISO/Linux/CentOS/kernel/kernel-firmware-2.6.32-504.3.3.el6.noarch.rpm
rpm -ivh http://soft.91yun.org/ISO/Linux/CentOS/kernel/kernel-2.6.32-504.3.3.el6.x86_64.rpm --force

# 查内核是否安装成功
rpm -qa | grep kernel

rm virt-what-1.12.tar.gz vm_check.sh
rm -rf virt-what-1.12

# 重启
reboot
