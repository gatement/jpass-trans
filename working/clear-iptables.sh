iptables -t filter -F
iptables -t filter -P FORWARD ACCEPT
iptables -t mangle -F
ip rule del fwmark 1 lookup 100
ip route del local 0.0.0.0/0 dev lo table 100
iptables -t mangle -L -n -v --line-numbers
