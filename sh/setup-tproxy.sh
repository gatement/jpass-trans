iptables -t filter -P FORWARD ACCEPT
iptables -t filter -F
iptables -t mangle -F
iptables -t mangle -A PREROUTING -p tcp -d 192.168.56.0/24 -j ACCEPT
iptables -t mangle -A PREROUTING -p tcp -d 192.168.1.1/32 -j ACCEPT
iptables -t mangle -A PREROUTING -p tcp -j TPROXY --on-port 8117 --tproxy-mark 1

ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
#ip route add local 0.0.0.0/0 dev lo tab local

#iptables -t mangle -L -n -v --line-numbers

#ip rule add fwmark 0x2333/0x2333 pref 101 table 101
#ip route add local default dev lo table 101
#iptables -t mangle -A PREROUTING -p udp -j TPROXY --tproxy-mark 0x2333/0x2333 --on-ip 127.0.0.1 --on-port 1080
