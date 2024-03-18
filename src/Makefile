all: main
main: main.cpp proxy.cpp proxy.h request.cpp request.h response.cpp response.h logger.h logger.cpp cache.cpp cache.h timer.hpp
		g++ -g -o main main.cpp proxy.cpp proxy.h request.cpp request.h response.cpp response.h logger.h logger.cpp cache.cpp cache.h timer.hpp -lpthread
.PHONY:
		cliean
clean:
		rm -rf *.o main

