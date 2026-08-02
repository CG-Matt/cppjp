.PHONY: all test lib static_lib dynamic_lib clean

CC		= g++
CFLAGS	= -Wall -Wextra -Iinclude
LFLAGS	=

SRCDIR	= src
BLDDIR	= build
INCDIR	= include
SHRDIR	= shared
STCDIR  = static

SRC		= $(wildcard $(SRCDIR)/*.cpp)
OBJ		= $(patsubst $(SRCDIR)/%.cpp,$(BLDDIR)/$(STCDIR)/%.o,$(SRC))
SHROBJ	= $(patsubst $(SRCDIR)/%.cpp,$(BLDDIR)/$(SHRDIR)/%.o,$(SRC))
INC		= $(wildcard $(INCDIR)/*.hpp) $(wildcard $(SRCDIR)/*.hpp)

all: test lib
test: $(BLDDIR)/cppjp-test
lib: static_lib dynamic_lib
static_lib: $(BLDDIR)/libcppjp.a
dynamic_lib: $(BLDDIR)/libcppjp.so

$(BLDDIR)/cppjp-test: $(OBJ) $(INC) | $(BLDDIR)
	$(CC) $(CFLAGS) -Isrc $(LFLAGS) -o $@ $(OBJ) cppjp.cpp

$(BLDDIR)/$(SHRDIR)/%.o: $(SRCDIR)/%.cpp $(INC) | $(BLDDIR)/$(SHRDIR)
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(BLDDIR)/$(STCDIR)/%.o: $(SRCDIR)/%.cpp $(INC) | $(BLDDIR)/$(STCDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BLDDIR)/%.a: $(OBJ) | $(BLDDIR)
	ar rcs $@ $(OBJ)

$(BLDDIR)/%.so: $(SHROBJ) | $(BLDDIR)
	$(CC) $(CFLAGS) $(LFLAGS) -fPIC -shared -o $@ $(SHROBJ)

$(BLDDIR)/$(STCDIR): | $(BLDDIR)
	mkdir -p $@

$(BLDDIR)/$(SHRDIR): | $(BLDDIR)
	mkdir -p $@

$(BLDDIR):
	mkdir -p $@

clean:
	rm -r build