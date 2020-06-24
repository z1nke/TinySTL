# CXX=clang++
SRC=./TinySTL/test.cpp

test: ${SRC}
	${CXX} ${CXXFLAGS} $< -o $@ -std=c++14

clean:
	rm test
