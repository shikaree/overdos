# ---------------------------------------------------------------------------
#  OverDOS emulator - portable build (Linux, macOS, *BSD, Windows/MinGW)
#
#    make          build ./overdos
#    make run      build and boot ./bootfile
#    make clean    remove build output
#
#  Only a C99 compiler and the C standard library are required - no external
#  dependencies.  Override the compiler or flags on the command line, e.g.
#    make CC=clang
# ---------------------------------------------------------------------------

CC      ?= cc
CFLAGS  ?= -O2 -Wall
SRC     := $(wildcard src/*.c)

ifeq ($(OS),Windows_NT)
  BIN := overdos.exe
else
  BIN := overdos
endif

.PHONY: all run clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

run: $(BIN)
	./$(BIN) bootfile

clean:
	rm -f overdos overdos.exe
