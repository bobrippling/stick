CXXFLAGS = -W -Wall -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wredundant-decls -Wextra -pedantic -ansi -Wabi -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Weffc++ -Wstrict-null-sentinel -Wno-non-template-friend -Wold-style-cast -Woverloaded-virtual -Wsign-promo
LDFLAGS  =

stick: socket.o main.o
	@echo LD $@
	@${CXX} ${LDFLAGS} -o $@ $^

%.o:%.cpp
	@echo CPP $<
	@${CXX} ${CXXFLAGS} -c -o $@ $<

clean:
	rm -f *.o stick

.PHONY: clean
