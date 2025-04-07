ARGS = -g -Wall -std=c++11

main: server client

server: httpServer.cpp http-server.o string-helpers.o http-server-context.o
	g++ $(ARGS) $^ -o server

client: httpClient.cpp http-client.o string-helpers.o
	g++ $(ARGS) $^ -o client

http-server.o: src/http-server.cpp
	g++ $(ARGS) -c $<

http-server-context.o: src/http-server-context.cpp
	g++ $(ARGS) -c $<

string-helpers.o: src/string-helpers.cpp
	g++ $(ARGS) -c $<

http-client.o: src/http-client.cpp
	g++ $(ARGS) -c $<

clean:
	rm server client *.o