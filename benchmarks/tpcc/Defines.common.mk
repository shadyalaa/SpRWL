CC       := g++
CFLAGS   += -std=c++11 -gdwarf-2 -g3 -w -pthread -mcpu=power8 -mtune=power8 -fpermissive
CFLAGS   += -O2
CFLAGS   += -I$(LIB)
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib
