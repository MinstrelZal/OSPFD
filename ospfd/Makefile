OBJ = lsr.o		\
      network.o		\
      interface.o	\
      neighbor.o	\
      area.o		\
      hello.o		\
      spf.o		\
      dd.o		\
      lsack.o		\
      ospfd.o		\
      lsa.o		\
      lsu.o		\
      route.o

TARGET = ospfd

.SUFFIXES:
.SUFFIXES: .c .o

CC = gcc
LIBS = -lpthread
CFLAGS = -std=gnu99 -Wall -g

all: $(TARGET)

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

.PHONY: clean

clean:
	-rm -f $(OBJ) $(TARGET) 
