iptables -t filter -F
iptables -t filter -P FORWARD ACCEPT
iptables -t mangle -F
#iptables -t mangle -D DIVERT
#iptables -t mangle -N DIVERT
#iptables -t mangle -A PREROUTING -p tcp -m socket -j DIVERT
#iptables -t mangle -A DIVERT -j MARK --set-mark 1
#iptables -t mangle -A DIVERT -j ACCEPT
iptables -t mangle -A PREROUTING -p tcp ! -d 192.168.0.0/16 -j TPROXY --on-port 8117 --tproxy-mark 1
#iptables -t mangle -A PREROUTING -p tcp -i enp0s9 -j TPROXY --tproxy-mark 0x1/0x1 --on-port 8117
iptables -t mangle -L -n -v --line-numbers
#ip rule del lookup 100
#ip route del local 0.0.0.0/0 dev lo table 100
#ip rule add fwmark 1 lookup 100
#ip route add local 0.0.0.0/0 dev lo table 100
#ip rule list
#ip route list
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100

