## config
<pre>
    vim /etc/sysctl.conf       
        net.ipv4.ip_forward = 1
    sysctl -p
    (or echo 1 > /proc/sys/net/ipv4/ip_forward)

    iptables -t filter -F
    iptables -t filter -P FORWARD ACCEPT
    iptables -t mangle -F
    iptables -t mangle -A PREROUTING -p tcp ! -d 192.168.0.0/16 -j TPROXY --on-port 8117 --tproxy-mark 1
    iptables -t mangle -L -n -v --line-numbers

    ip rule add fwmark 1 lookup 100
    ip route add local 0.0.0.0/0 dev lo table 100
    //ip route add local 0.0.0.0/0 dev lo tab local

    ip rule del fwmark 1 lookup 100
    ip route del local 0.0.0.0/0 dev lo table 100
    //ip route del local 0.0.0.0/0 dev lo tab local
</pre>

## dependency
yum install -y libssh2-devel

## compile
make
make clean

## run
bin/jpass-trans
