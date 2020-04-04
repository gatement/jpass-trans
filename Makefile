objects = main.o util.o
output = bin
compiler = cc

compile: $(objects) mkoutput
	$(compiler) -o $(output)/jpass-trans -lssh2 $(objects)

main.o: main.h
util.o: util.h

.PHONY: mkoutput clean help

mkoutput:
	mkdir -p $(output)

clean:
	rm -rf $(output) *.o

help:
	@echo "make \
       \nmake clean"
