CC = g++

COMPILER_FLAGS = -std=c++20

all : central_server.cpp
	$(CC) server.cpp -o  sv.out $(COMPILER_FLAGS)
	$(CC) central_server.cpp -o  centr.out $(COMPILER_FLAGS)
	$(CC) client.cpp -o  cli.out $(COMPILER_FLAGS)

clean:
	rm -rf *.out

re : clean all

.PHONY: all clean re