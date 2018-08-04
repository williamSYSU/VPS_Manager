#!/bin/bash

opt=$1
ip=$2
target=$3

if [ $opt != "I" -a $opt != "D" ]; then
    echo "非法操作，输入：I或D"
    exit 1
fi

if [ $target != "REJECT" -a $target != "ACCEPT" ]; then
    echo "非法目标，输入：REJECT或ACCEPT"
    exit 1
fi

iptables -$opt INPUT -s $ip -j $target

if [ ! -d "/etc/iptables-rules" ]; then
    mkdir /etc/iptables-rules
fi
iptables-save > /etc/iptables-rules/rules.ban
cp -f /etc/iptables-rules/rules.ban /root/vps-server/rules.ban

echo "execute: iptables -$opt INPUT -s $ip -j $target"

