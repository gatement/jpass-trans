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
client <==> tproxy <=TCP=> jpass-trans <=TCP=> jpass-ssh <==> sshd <=TCP=> host

## arch (UDP)
client <==> tproxy <=UDP=> jpass-trans <=TCP1=> jpass-ssh <==> sshd <=TCP2=> jpass-dns <=UDP=> DNSServer
  TCP1,TCP2 means same TCP connnection
  TCP1 carry client UDP ip:port
