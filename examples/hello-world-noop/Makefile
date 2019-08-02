.PHONY: hello clean

hello: main.cpp mylib.c mylib.h
	$(CXX) -std=c++17 $(CFLAGS) -Wall main.cpp mylib.c -o $@

clean:
	-rm hello
