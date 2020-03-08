

objects += socket_server.cpp threadpool.cpp communication.cpp
compiler += arm-linux-gnueabihf-

compileopt = -std=c++11 -lpthread -DMEM_DEBUG

main: $(objects)
	$(compiler)g++ $(objects) $(compileopt) -o  SERVER

.PHONY:clean

clean:
	rm SERVER
