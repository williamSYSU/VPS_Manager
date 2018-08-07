#!/bin/bash

opt=$1
ip=$2

if [ $opt != "I" -a $opt != "D" ]; then
    echo "非法操作，输入：I或D"
    exit 1
fi

iptables -$opt INPUT -s $ip -j REJECT 

if [ ! -d "/etc/iptables-rules" ]; then
    mkdir /etc/iptables-rules
fi
iptables-save > /etc/iptables-rules/rules.ban
cp -f /etc/iptables-rules/rules.ban /root/vps-server/rules.ban

echo "execute: iptables -$opt INPUT -s $ip -j REJECT"

