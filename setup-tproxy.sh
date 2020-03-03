iptables -t filter -P FORWARD ACCEPT
iptables -t mangle -A PREROUTING -p tcp ! -d 192.168.0.0/16 -j TPROXY --on-port 8117 --tproxy-mark 1

ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100

iptables -t mangle -L -n -v --line-numbers
