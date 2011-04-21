CFLAGS   = -Wall -Wextra -pedantic -O2 -g -D_POSIX_SOURCE
CXXFLAGS = ${CFLAGS}
LDFLAGS  = -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx

all: check Stick

Stick: \
		stick.o \
		obj.o \
		bullet.o \
		\
		files.o \
		gfx.o \
		util.o \
		main.o \
		global.o \
		\
		net/addr.o \
		net/udp_socket.o
	${CXX} ${LDFLAGS} -o $@ $^

%.o:%.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

%.o:%.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f `find -iname \*.o` Stick check

check: check.cpp
	${CC} ${CFLAGS} -o $@ $<
	./$@

.PHONY: clean

# :r!g++ -MM {net/,}*.cpp

addr.o: net/addr.cpp net/addr.h
tcp_socket.o: net/tcp_socket.cpp net/tcp_socket.h
udp_socket.o: net/udp_socket.cpp net/addr.h net/udp_socket.h
bullet.o: bullet.cpp config.h gfx.h 2d.h common.h bullet.h
files.o: files.cpp files.h util.h
gfx.o: gfx.cpp config.h gfx.h files.h net/headers.h net/addr.h \
 net/udp_socket.h 2d.h common.h obj.h stick.h bullet.h plat.h global.h
global.o: global.cpp config.h net/headers.h net/addr.h net/udp_socket.h \
 gfx.h 2d.h common.h obj.h stick.h bullet.h plat.h util.h files.h \
 global.h
main.o: main.cpp config.h net/headers.h net/addr.h net/udp_socket.h gfx.h \
 2d.h common.h obj.h stick.h bullet.h plat.h util.h files.h global.h
obj.o: obj.cpp config.h gfx.h 2d.h common.h obj.h
stick.o: stick.cpp config.h net/headers.h net/addr.h gfx.h 2d.h common.h \
 obj.h stick.h files.h util.h
util.o: util.cpp util.h
