test:
CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -I. -I.. -O3

OBJECTS := pre_computed.o board.o evaluation.o main.o tuner.o

vpath %.cc ..
vpath main.cc .

main: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o main $(CXXFLAGS)

%.o: %.cc
	g++ -c $< -o $@ $(CXXFLAGS)

-include *.d

clean:
	rm -f *.o *.d
	rm main

test: main
	./main
