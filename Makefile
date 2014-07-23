# build helloworld executable when user executes "make"
all: pmaxd xplsendjson jsongetxplstate test testpmaxd
pmaxd: pmaxd.o
	$(CC) $(LDFLAGS) pmaxd.o -o pmaxd ../../libxpl/src/libxPL.so -lconfig
pmaxd.o: pmaxd.c pmax_constant.h
	$(CC) $(CFLAGS) -c pmaxd.c -I../../libxpl/src
xplsendjson: xplsendjson.o
	$(CC) $(LDFLAGS) xplsendjson.o -o xplsendjson.cgi -l../../libxpl/src/libxPL.so
xplsendjson.o: xplsendjson.c
	$(CC) $(CFLAGS) -c xplsendjson.c
jsongetxplstate: jsongetxplstate.o
	$(CC) $(LDFLAGS) jsongetxplstate.o -o jsongetxplstate.cgi -lxPL
jsongetxplstate.o: jsongetxplstate.c
	$(CC) $(CFLAGS) -c jsongetxplstate.c
test: test.o
	$(CC) $(LDFLAGS) test.o -o test -lxPL -lconfig
test.o: test.c
	$(CC) $(CFLAGS) -c test.c
testpmaxd: testpmaxd.o
	$(CC) $(LDFLAGS) testpmaxd.o -o testpmaxd -lxPL -lconfig
testpmaxd.o: testpmaxd.c
	$(CC) $(CFLAGS) -c testpmaxd.c
		
	
	

# remove object files and executable when user executes "make clean"
clean:
	rm *.o pmaxd testpmaxd xplsendjson.cgi jsongetxplstate.cgi
