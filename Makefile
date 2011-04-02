CFLAGS   = -Wall -Wextra -pedantic -g -D_POSIX_SOURCE
CXXFLAGS = ${CFLAGS}
LDFLAGS  = -lSDL -lSDL_image

Stick: stick.o files.o gfx.o util.o main.o \
		net/addr.o net/udp_socket.o global.o
	${CXX} ${LDFLAGS} -o $@ $^

%.o:%.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

%.o:%.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f `find -iname \*.o` Stick

.PHONY: clean

stick.o: stick.cpp net/headers.h net/addr.h gfx.h stick.h
files.o: files.cpp files.h util.h
gfx.o: gfx.cpp gfx.h files.h
main.o: main.cpp net/headers.h net/addr.h net/udp_socket.h gfx.h stick.h \
 util.h
util.o: util.cpp util.h
addr.o: net/addr.cpp net/addr.h
tcp_socket.o: net/tcp_socket.cpp net/tcp_socket.h
udp_socket.o: net/udp_socket.cpp net/addr.h net/udp_socket.h
