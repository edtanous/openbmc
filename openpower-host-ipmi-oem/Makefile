CXX ?= $(CROSS_COMPILE)g++

TESTER      = testit
TESTER_OBJ  = testit.o oemhandler.o

LIB_OEM_OBJ = oemhandler.o
LIB_OEM     = liboemhandler.so

LDFLAGS += -rdynamic -ldl
CXXFLAGS += -fPIC -Wall


all:  $(TESTER) $(LIB_OEM)

%.o: %.C
	$(CXX) -c $< $(CXXFLAGS) -o $@


$(LIB_OEM): $(LIB_OEM_OBJ)
	$(CXX) $^ -shared $(LDFLAGS) -o $@

$(TESTER): $(TESTER_OBJ)
	$(CXX) $^ $(LDFLAGS) $(LIB_FLAG) -o $@ -ldl

clean:
	$(RM) $(LIB_OEM_OBJ) $(LIB_OEM) $(TESTER_OBJ) $(TESTER)
