ARGS = -g -Wall -std=c++11

main: http-server.o http-server-context.o string-helpers.o http-client.o

http-server.o: src/http-server.cpp
	g++ $(ARGS) -c $<

http-server-context.o: src/http-server-context.cpp
	g++ $(ARGS) -c $<

string-helpers.o: src/string-helpers.cpp
	g++ $(ARGS) -c $<

http-client.o: src/http-client.cpp
	g++ $(ARGS) -c $<

clean:
	rm *.o