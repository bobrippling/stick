CXXFLAGS = -g -W -Wall -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wredundant-decls -Wextra -pedantic -ansi -Wabi -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Weffc++ -Wno-non-template-friend -Woverloaded-virtual -Wsign-promo
LDFLAGS  =

stick: tcp_socket.o udp_socket.o socket.o main.o
	@echo LD $@
	@${CXX} ${LDFLAGS} -o $@ $^

%.o:%.cpp
	@echo CPP $<
	@${CXX} ${CXXFLAGS} -c -o $@ $<

%.o:%.c
	@echo CC $<
	@${CC} ${CFLAGS} -c -o $@ $<

clean:
	@rm -f *.o stick

.PHONY: clean

main.o: main.cpp socket.h
tcp_socket.o: tcp_socket.cpp tcp_socket.h socket.h
udp_socket.o: udp_socket.cpp udp_socket.h socket.h
socket.o: socket.c socket.h
