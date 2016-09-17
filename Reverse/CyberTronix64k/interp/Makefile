run: interp
	./interp ../t

clean:
	rm ./interp

interp: interp.cpp memory.cpp memory.h
	gcc --std=c++11 interp.cpp memory.cpp -o interp
