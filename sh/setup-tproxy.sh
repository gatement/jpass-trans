#> vim /etc/sysctl.conf
#    net.ipv4.ip_forward = 1
#> sysctl -p
#> cat /proc/sys/net/ipv4/ip_forward

iptables -t filter -P FORWARD ACCEPT
iptables -t filter -F
iptables -t mangle -F
iptables -t mangle -A PREROUTING -p tcp -d 192.168.1.0/24 -j ACCEPT
iptables -t mangle -A PREROUTING -p tcp -j TPROXY --on-port 8116 --tproxy-mark 1

ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
#ip route add local 0.0.0.0/0 dev lo tab local

#iptables -t mangle -L -n -v --line-numbers
