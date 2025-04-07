ARGS = -g -Wall -std=c++11

main: cas-lib server client

server: httpServer.cpp
	g++ $(ARGS) $< cas/http-server.o cas/string-helpers.o cas/http-server-context.o -o server

client: httpClient.cpp
	g++ $(ARGS) $< cas/http-client.o cas/string-helpers.o -o client

cas-lib:
	make -C ./cas -f makelib.mk

clean:
	rm server client *.o | make -C ./cas -f makelib.mk clean