#iptables -F -t nat
#iptables -t nat -A PREROUTING -d 10.33.176.106 -p tcp --dport 10000 -j REDIRECT --to-ports 10001
#iptables -t nat -L -nv  --line-numbers

iptables -F -t nat
iptables -t nat -A PREROUTING -d 10.33.176.106 -p tcp --dport 10000 -j DNAT --to-destination 10.33.176.104:10001
iptables -t nat -A POSTROUTING -d 10.33.176.104 -p tcp --dport 10001 -j SNAT --to-source 10.33.176.106
iptables -t nat -L -nv  --line-numbers

##process:
##  client => proxy: sync request
##  10.33.176.32:12345 -> 10.33.176.106:10000   PREROUTING  => 
##  10.33.176.32:12345 -> 10.33.176.104:10001   POSTROUTING =>
##  proxy => server: sync created
##  10.33.176.106:12345 -> 10.33.176.104:10001
##  server => proxy:
##  10.33.176.104:10001 -> 10.33.176.106:12345

