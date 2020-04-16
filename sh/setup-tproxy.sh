#> vim /etc/sysctl.conf
#    net.ipv4.ip_forward = 1
#> sysctl -p
#> cat /proc/sys/net/ipv4/ip_forward

iptables -t filter -P FORWARD ACCEPT
iptables -t filter -F
iptables -t mangle -F
# ssh
iptables -t mangle -A PREROUTING -p tcp -d 192.168.56.0/24 -j ACCEPT
iptables -t mangle -A PREROUTING -p udp -d 192.168.56.0/24 -j ACCEPT
# gateway
iptables -t mangle -A PREROUTING -p tcp -d 10.0.3.2/32 -j ACCEPT
iptables -t mangle -A PREROUTING -p udp -d 10.0.3.2/32 -j ACCEPT
# tproxy
iptables -t mangle -A PREROUTING -p tcp -j TPROXY --on-port 8116 --tproxy-mark 1
iptables -t mangle -A PREROUTING -p udp -j TPROXY --on-port 53 --tproxy-mark 1
#iptables -t mangle -A PREROUTING -p udp -j TPROXY --on-port 53 --tproxy-mark 0x2333/0x2333 

# tcp
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
#ip route add local 0.0.0.0/0 dev lo tab local

# udp
#ip rule add fwmark 0x2333/0x2333 pref 101 table 101
#ip route add local default dev lo table 101

#ip rule add fwmark 0x2333/0x2333 pref 101 table 101
#ip route add local default dev lo table 101
#iptables -t mangle -A PREROUTING -p udp -j TPROXY --tproxy-mark 0x2333/0x2333 --on-ip 127.0.0.1 --on-port 1080

#iptables -t mangle -L -n -v --line-numbers
