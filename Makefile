CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -lm
BIN_DIR := ./bin

WKDM_LIB := deps/WKdm/WKdm.o
LZ4_LIB  := deps/lz4/lib/liblz4.a
ZLIB_LIB := deps/zlib-1.3.2/libz.a

.PHONY: all WKdm lz4 zlib clean

all: WKdm lz4 zlib

WKdm: $(BIN_DIR)/WKdm
$(BIN_DIR)/WKdm: main.c adapters/WKdm_adapter.h
	$(CC) $(CFLAGS) -DWKdm -o $@ main.c $(WKDM_LIB)

lz4: $(BIN_DIR)/lz4
$(BIN_DIR)/lz4: main.c adapters/lz4_adapter.h
	$(CC) $(CFLAGS) -Dlz4 -o $@ main.c $(LZ4_LIB)

zlib: $(BIN_DIR)/zlib
$(BIN_DIR)/zlib: main.c adapters/zlib_adapter.h
	$(CC) $(CFLAGS) -Dzlib -o $@ main.c $(ZLIB_LIB)

clean:
	rm -rf $(BIN_DIR)