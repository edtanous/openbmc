CXX ?= $(CROSS_COMPILE)g++

LIB_OEM_OBJ = oemhandler.o
LIB_OEM     = liboemhandler.so

LDFLAGS += -rdynamic -ldl
CXXFLAGS += -fPIC -Wall -Werror


all:  $(LIB_OEM)

%.o: %.C
	$(CXX) -std=c++14 -c $< $(CXXFLAGS) -o $@

$(LIB_OEM): $(LIB_OEM_OBJ)
	$(CXX) $^ -shared $(LDFLAGS) -o $@

clean:
	$(RM) $(LIB_OEM_OBJ) $(LIB_OEM)
