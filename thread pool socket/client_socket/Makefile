

objects += socket_client.cpp communication.cpp
compiler += #arm-linux-gnueabihf-

compileopt = -std=c++11 -lpthread -DMEM_DEBUG

main: $(objects)
	$(compiler)g++ $(objects) $(compileopt) -o  CLIENT

.PHONY:clean

clean:
	rm CLIENT
