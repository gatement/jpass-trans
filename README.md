## dependency
please use jpass-ssh as backend

## linux compile
make tcp
make udp
make clean

## openwrt cross compile
./build-openwrt.sh 

## run
bin/jpass-trans-tcp
bin/jpass-trans-udp

## arch (TCP)
detail: client <==> tproxy <=TCP=> jpass-trans <=TCP=> jpass-ssh <==> sshd <=TCP=> host

## arch (UDP)
client <==> tproxy <=UDP=> jpass-trans <=TCP=> jpass-ssh <==> sshd <=TCP=> jpass-dns <=UDP=> DNSServer
