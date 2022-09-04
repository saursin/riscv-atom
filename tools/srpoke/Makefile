$(shell mkdir -p bin)
$(shell mkdir -p obj)

SIMDIR := ../../sim
LIBPATH := $(SIMDIR)/include

CFLAGS := -Wall -I $(LIBPATH)
LDLIBS := -L $(LIBPATH)/CppLinuxSerial

LDFLAGS := -lCppLinuxSerial -pthread

default: bin/srpoke

bin/srpoke: obj/vuart.o obj/util.o obj/srpoke.o
	g++ $(LDLIBS) -o $@ $^ $(LDFLAGS)

obj/vuart.o: $(SIMDIR)/vuart.cpp
	g++ -c $(CFLAGS) -o $@ $^

obj/util.o: $(SIMDIR)/util.cpp
	g++ -c $(CFLAGS) -o $@ $^

obj/srpoke.o: srpoke.cpp
	g++ -c $(CFLAGS) -o $@ $^


clean:
	rm -rf bin/* obj/*