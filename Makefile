CXXFLAGS = -g -W -Wall -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wredundant-decls -Wextra -pedantic -ansi -Wabi -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Weffc++ -Wno-non-template-friend -Woverloaded-virtual -Wsign-promo
LDFLAGS  =

stick: socket.o main.o
	@echo LD $@
	@${CXX} ${LDFLAGS} -o $@ $^

%.o:%.cpp
	@echo CPP $<
	@${CXX} ${CXXFLAGS} -c -o $@ $<

clean:
	@rm -f *.o stick

.PHONY: clean

main.o: main.cpp socket.h
socket.o: socket.cpp socket.h
