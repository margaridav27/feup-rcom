#!/bin/sh

#tux3

ifconfig eth0 down
ifconfig eth0 up 172.16.10.1/24

route add -net 172.16.11.0/24 gw 172.16.10.254

arp -a

route add default gw 172.16.10.254

route -n