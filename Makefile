MAKEFLAGS += -j8
test:

CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3 -flto -I.
BMI2FLAGS   = -march=x86-64 -mpopcnt -msse -msse2 -mssse3 -msse4.1 -mavx2 -mbmi -mbmi2
AVX2FLAGS   = -march=x86-64 -mpopcnt -msse -msse2 -mssse3 -msse4.1 -mavx2 -mbmi
POPCNTFLAGS = -march=x86-64 -mpopcnt

OBJECTS := main.o pre_computed.o search.o board.o evaluation.o move_generator.o uci.o

priessnitz: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o priessnitz $(CXXFLAGS)

%.o: %.cc
	g++ -c $< -o $@ $(CXXFLAGS)

-include *.d

clean:
	rm -f *.o *.d
	rm priessnitz

test: priessnitz
	./priessnitz

profile: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o priessnitz $(CXXFLAGS) -g -pg 
	./priessnitz
	gprof priessnitz | gprof2dot | dot -Tpng -o profile.png

release:
	g++ *.cc -o priessnitz_1.0_bmi2   $(CXXFLAGS) $(BMI2FLAGS)
	g++ *.cc -o priessnitz_1.0_avx2   $(CXXFLAGS) $(AVX2FLAGS)
	g++ *.cc -o priessnitz_1.0_popcnt $(CXXFLAGS) $(POPCNTFLAGS)
