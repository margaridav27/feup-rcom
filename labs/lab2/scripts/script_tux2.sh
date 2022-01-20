#!/bin/sh

#tux2

ifconfig eth0 down
ifconfig eth0 up 172.16.11.1/24

route add -net 172.16.10.0/24 gw 172.16.11.253

arp -a

route add default gw 172.16.11.254

route -n