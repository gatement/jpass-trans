## dependency
please use jpass-ssh as backend

## linux compile
make
make clean

## openwrt cross compile
./build-openwrt.sh 

## run
sudo sh/clear-tproxy.sh
sudo sh/setup-tproxy.sh
sudo bin/jpass-trans 8116 192.168.1.2 8117
