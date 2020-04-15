objects_tcp = main_tcp.o util.o
objects_udp = main_udp.o util.o
output = bin
compiler = cc

tcp: $(objects_tcp) mkoutput
	$(compiler) -o $(output)/jpass-trans-tcp $(objects_tcp)

udp: $(objects_udp) mkoutput
	$(compiler) -o $(output)/jpass-trans-udp $(objects_udp)

util.o: util.h

.PHONY: mkoutput clean help

mkoutput:
	mkdir -p $(output)

clean:
	rm -rf $(output) *.o

help:
	@echo "make tcp|upd|clean"
