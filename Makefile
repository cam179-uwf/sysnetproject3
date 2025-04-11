ARGS = -g -Wall -std=c++11

main: cas-lib server client

test1: tests/test1.cpp 
	g++ $(ARGS) $< cas/http-server.o cas/string-helpers.o cas/http-server-context.o cas/http-client.o -o test1
	./test1

server: httpServer.cpp helpers.o
	g++ $(ARGS) $^ cas/http-server.o cas/string-helpers.o cas/http-server-context.o -o server

client: httpClient.cpp
	g++ $(ARGS) $< cas/http-client.o cas/string-helpers.o -o client

helpers.o: src/helpers.cpp
	g++ $(ARGS) -c $<

cas-lib:
	make -C ./cas -f makelib.mk

clean:
	make -C ./cas -f makelib.mk clean
	rm server client test1 *.o