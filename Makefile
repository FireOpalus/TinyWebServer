all:
	mkdir -p bin
	cd build && make -j4

clean:
	rm -rf bin