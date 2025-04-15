# Weather Alert System 1.2

### Table of contents:
- [About](#about)
- [How to comiple](#how-to-compile)
- [How to run](#how-to-run)

## About

This project includes a Client/Server library that is held in the client and server namespace (cas). This library is independently compiled through its own make file. The make file for the cas library is called by the root project's make file.

After the server is started. You may start the client(s).

The started client will display a list of options when not logged in.

```
1: register
2: log in
0: quit
Pick an option or type (exit):
```

Once logged in the client will display this list of options:

```
1: log out
2: change password
3: subscribe to a location
4: unsubscribe from a location
5: get locations
0: quit
Pick an option or type (exit):
```

The connection between the server and client is persistant until the client disconnects. However, in the case where a client wants to use multiple ports from the server, a bearer token is given to the client so as to authenticate from any port. This also allows the client to disconnect and then reconnect while maintaining their session.

The server will not log much except some verious scattered events.

A list of all of the registered users is stored within the `users.txt` file.

This project also includes a tests folder, but no tests are actually created yet. Also, this project has a web page that you can load at `localhost:60001`. Using this web page you can send messages to the server. The web page is there just for fun.

## How to compile

**This project was built and tested on the Windows Subsystem for Linux (WSL) platform.** Please use WSL for best test results.

Run the make file found in the root directory by executing `make`.

This should produce two files, a `server` executable and a `client` executable.

## How to run

The `server` executable can be run by executing the command `./server` from this root directory.

The `client` executable can be run by executing the command `./client` from this root directory.

The server will automatically start listening to port `60001`. This is configurable through code only. If changing the port is desired, run `make clean` from the root directory. Then go to `libs/weather-service.hpp` and change the call to the `HttpServer` constructor to instead use the desired port number. After the change is complete. Run `make` again.

The client is similar. It is set up to connect to `localhost:60001`. This is only configurable through code. If a change of address is desired. Repeat the steps for changing the server's port, but instead of changing the *weather-service.hpp* file, go to the `httpClient.cpp` file and change the call to the `HttpClient` constructor to instead use the desired address.

---

&copy; 2025 Christian Marcellino and Ryan Waddington