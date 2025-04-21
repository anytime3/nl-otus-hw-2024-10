.PHONY: all clean

all: bin/mapper bin/reducer

bin/mapper:
	mkdir -p bin
	g++ -o  mapper.cpp

bin/reducer:
	mkdir -p bin
	g++ -o  reducer.cpp

clean:
	rm -rf bin output
