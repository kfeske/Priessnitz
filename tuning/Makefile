test:
CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3 -I../include
-include *.d

main: Makefile
main:
	g++ tuner.cc -o main $(CXXFLAGS)



clean:
	rm -f *.o *.d
	rm main

test: main
	./main
