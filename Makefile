ARGS = -g -Wall -std=c++11

main: main.cpp http-server.o string-helpers.o http-server-context.o
	g++ $(ARGS) $^ -o main

http-server.o: src/http-server.cpp
	g++ $(ARGS) -c $<

http-server-context.o: src/http-server-context.cpp
	g++ $(ARGS) -c $<

string-helpers.o: src/string-helpers.cpp
	g++ $(ARGS) -c $<

clean:
	rm main *.o