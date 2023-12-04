# Experience 2 - Part 3 Configure a Router in Linux

## Steps

1. Connect and configure E1 from Tux34 to the switch:
```bash
ifconfig eth1 up
ifconfig eth1 172.16.31.253/24
```

2. Connect Tux34.eth1 to bridge31
```bash
/interface bridge port remove [find interface=ether4]
/interface bridge port add interface=ether4 bridge=bridge31
```

3. Check MAC and IP addresses in Tux34.eth1 and Tux34.eth0
```
ifconfig
MAC eth0 -> 00:21:5a:5a:7d:74
IP eth0 -> 172.16.30.254
Mac eth1 -> 00:c0:df:25:24:5b
IP eth1 -> 172.16.31.253 
```

4. Reconfigure TuxY3 and TuxY2 so that each of them can reach the other
```bash
route add deafult gw 172.16.30.254 -> TuxY3

route add default gw 172.16.31.253 -> TuxY2
```

5. Verify the gateways on all machines:
```
route -n
```

6. Activate ip forwarding on Tux34 and ignore broadcasts on all machines:
```bash
sysctl net.ipv4.ip_forward=1
sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
```

6. Start capture at Tux33

7. From Tux33, ping the other network interfaces and save the logs:
```
ping 172.16.30.254
ping 172.16.31.253
ping 172.16.31.1
```

8. Start capture of eth0 and eth1 in Tux34

9. Clean the ARP tables on all machines:
```
arp -d 172.16.31.253
arp -d 172.16.30.254 
arp -d 172.16.30.1 
arp -d 172.16.31.1 
```

10. Ping Tux32 from Tux33 and save the logs.

## Questions

What are the commands required to configure this experience?
```
ifconfig eth1 up
ifconfig eth1 172.16.31.253/24
/interface bridge port remove [find interface=ether4]
/interface bridge port add interface=ether4 bridge=bridge31
/interface bridge port print brief
route add deafult gw 172.16.30.254 -> TuxY3
route add default gw 172.16.31.253 -> TuxY2
sysctl net.ipv4.ip_forward=1
sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
ping command
arp -d
```

What routes are there in the tuxes? What are their meaning?
```
Tux32 and Tux33 both have 1 route and they have Tux34's IP as a gateway because this machine belongs to both bridges.
```

What information does an entry of the forwarding table contain?
```
Each entry contains a destination address and its respective gateway.
```

What ARP messages, and associated MAC addresses, are observed and why?

What ICMP packets are observed and why? 

What are the IP and MAC addresses associated to ICMP packets and why? 