## dependency
please use jpass-ssh as backend

## linux compile
make tcp
make udp
make clean

## openwrt cross compile
./build-openwrt.sh 

## run
TCP tunnel: bin/jpass-trans-tcp 8117
UDP tunnel: bin/jpass-trans-udp udp 8116
DNS tunnel: bin/jpass-trans-udp dns 53

## arch (TCP)
client <==> tproxy <=TCP=> jpass-trans <=TCP=> jpass-ssh <==> sshd <=TCP=> host
 
## arch (UDP)
client <==> tproxy <=UDP=> jpass-trans <=TCP=> jpass-ssh <==> sshd <=TCP=> jpass-udp <=UDP=> host
