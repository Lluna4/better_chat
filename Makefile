CC = g++

COMPILER_FLAGS = -std=c++20

all : central_server.cpp
	$(CC) server.cpp -o  output/sv.out $(COMPILER_FLAGS)
	$(CC) central_server.cpp -o  output/centr.out $(COMPILER_FLAGS)
	$(CC) term_client.cpp -o  output/term_cli.out  $(COMPILER_FLAGS)
	$(CC) client.cpp -o output/cli.out -std=c++23 -lncurses

clean:
	rm -rf *.out

re : clean all

.PHONY: all clean re