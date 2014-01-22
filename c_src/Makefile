.PHONY: clean
.SUFFIXES: .o .c .cc

OS= ${shell uname}
CC=gcc
CXX=g++
CXXFLAGS=-Wall -g

# Erlang
ERL_INCLUDE = -I/usr/local/lib/erlang/usr/include
ERL_LIBS    = -L/usr/local/lib/erlang/usr/lib \
              -lerts
EI_INCLUDE  = -I/usr/local/lib/erlang/lib/erl_interface-3.7.15/include
EI_LIBS     = -L/usr/local/lib/erlang/lib/erl_interface-3.7.15/lib \
              -lei \
              -lerl_interface

ifeq ($(OS), Darwin)
  EI_LIBS  += -framework OpenAL
else
  EI_LIBS  += -lopenal
endif

TARGET_LIB = playerlang_drv.so
ifeq ($(OS), Darwin)
  EXTRA_OPTIONS = -fno-common -bundle -undefined suppress -flat_namespace
endif
ALL: $(TARGET_LIB)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CXXFLAGS) $(ERL_INCLUDE) $(EI_INCLUDE) -c $<

playerlang_drv.so: playerlang_drv.o
	$(CXX) -o $@ $^ $(ERL_LIBS) $(EI_LIBS) $(EXTRA_OPTIONS) -fpic -O2
	mv playerlang_drv.so ../ebin

clean:
	rm -f *.o *.so
