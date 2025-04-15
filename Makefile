ARGS = -g -Wall -std=c++11
LIBS = cas/http-server.o cas/string-helpers.o cas/http-request.o cas/http-response.o cas/http-client.o cas/fd-reader.o

main: cas-lib server client

test1: tests/test1.cpp 
	g++ $(ARGS) $< $(LIBS) -o test1
	./test1

server: httpServer.cpp helpers.o user-info.o weather-service.o
	g++ $(ARGS) -pthread $^ $(LIBS) -o server

client: httpClient.cpp
	g++ $(ARGS) -pthread $< $(LIBS) -o client

helpers.o: src/helpers.cpp
	g++ $(ARGS) -c $<

user-info.o: src/user-info.cpp
	g++ $(ARGS) -c $<

weather-service.o: src/weather-service.cpp
	g++ $(ARGS) -c $<

cas-lib:
	make -C ./cas

clean:
	make -C ./cas clean
	rm server client test1 *.o