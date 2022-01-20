configure terminal
interface FastEthernet0/0
 ip address 172.16.2.19 255.255.255.0
 ip nat outside
 no shutdown

interface FastEthernet0/1
 ip address 172.16.11.254 255.255.255.0
 ip nat inside
 no shutdown

ip route 0.0.0.0 0.0.0.0 172.16.2.254
ip route 172.16.10.0 255.255.255.0 172.16.11.2

ip nat pool ovrld 172.16.2.19 172.16.2.19 prefix-length 24
ip nat inside source list 1 pool ovrld overload

access-list 1 permit 172.16.10.0 0.0.0.7
access-list 1 permit 172.16.11.0 0.0.0.7
