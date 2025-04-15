ARGS = -g -Wall -std=c++11

main: http-server.o http-request.o http-response.o string-helpers.o http-client.o fd-reader.o

http-server.o: src/http-server.cpp
	g++ $(ARGS) -c $<

http-request.o: src/http-request.cpp
	g++ $(ARGS) -c $<

http-response.o: src/http-response.cpp
	g++ $(ARGS) -c $<

string-helpers.o: src/string-helpers.cpp
	g++ $(ARGS) -c $<

http-client.o: src/http-client.cpp
	g++ $(ARGS) -c $<

fd-reader.o: src/fd-reader.cpp
	g++ $(ARGS) -c $<

clean:
	rm *.o