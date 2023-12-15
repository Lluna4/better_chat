CC = g++

COMPILER_FLAGS = -std=c++20

all : src/central_server/central_server.cpp
	$(CC) src/server/server.cpp -o  output/sv.out $(COMPILER_FLAGS)
	$(CC) src/central_server/central_server.cpp -o  output/centr.out $(COMPILER_FLAGS)
	$(CC) src/client/term_client.cpp -o  output/term_cli.out  $(COMPILER_FLAGS)
	$(CC) src/client/client.cpp -o output/cli.out -std=c++23 -lncurses

clean:
	rm -rf output/*.out

re : clean all

.PHONY: all clean re