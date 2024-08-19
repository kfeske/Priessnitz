MAKEFLAGS += -j8
test:

CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3 -march=native
#CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3

OBJECTS := main.o pre_computed.o search.o board.o evaluation.o move_generator.o uci.o

priessnitz: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o priessnitz $(CXXFLAGS)

profile: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o priessnitz $(CXXFLAGS) -g -pg 
	./priessnitz
	gprof priessnitz | gprof2dot | dot -Tpng -o profile.png

%.o: %.cc
	g++ -c $< -o $@ $(CXXFLAGS)

-include *.d

clean:
	rm -f *.o *.d
	rm priessnitz

test: priessnitz
	./priessnitz
