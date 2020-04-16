iptables -t mangle -F

ip rule del fwmark 1 lookup 100
ip route del local 0.0.0.0/0 dev lo table 100

ip rule del fwmark 0x2333/0x2333 pref 101 table 101
ip route del local default dev lo table 101

#iptables -t mangle -L -n -v --line-numbers
