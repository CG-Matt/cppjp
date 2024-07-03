.PHONY: all lib test static_lib dynamic_lib clean clean_build

CC = g++
FLAGS = -Wall -Wextra

SRCDIR = src
BLDDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(BLDDIR)/%.o, $(SOURCES))

all: test lib

test:
	$(CC) $(FLAGS) -o cppjp $(SOURCES) cppjp.cpp -Iinclude

lib: static_lib dynamic_lib

dynamic_lib:
	$(CC) $(FLAGS) -fPIC -shared -o libcppjp.so $(SOURCES) -Iinclude

static_lib: clean_build
	mkdir -p build
	cd build; $(CC) $(FLAGS) -c $(patsubst %, ../%, $(SOURCES)) -Iinclude
	ar rcs libcppjp.a $(OBJECTS)

clean: clean_build
	rm -f libcppjp.a libcppjp.so rm cppjp

clean_build:
	rm -rf build