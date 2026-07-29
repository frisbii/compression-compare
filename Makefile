CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -Wno-unused-parameter -Wno-maybe-uninitialized -lm 
BIN_DIR := ./bin

WKDM_LIB := deps/WKdm/WKdm.o
LZ4_LIB  := deps/lz4/lib/liblz4.a
ZLIB_LIB := deps/zlib-1.3.2/libz.a
WK64_LIB := deps/WK64/libwk.a
LZO_LIB := deps/lzo-2.10/src/.libs/liblzo2.a
ZSTD_LIB := deps/zstd/lib/libzstd.a

.PHONY: all WKdm lz4 zlib WK64 lzo zstd clean

all: WKdm lz4 zlib WK64 lzo zstd

WKdm: $(BIN_DIR)/WKdm
$(BIN_DIR)/WKdm: main.c adapters/WKdm_adapter.h
	$(CC) $(CFLAGS) -DWKdm -o $@ main.c $(WKDM_LIB)

lz4: $(BIN_DIR)/lz4
$(BIN_DIR)/lz4: main.c adapters/lz4_adapter.h
	$(CC) $(CFLAGS) -Dlz4 -o $@ main.c $(LZ4_LIB)

zlib: $(BIN_DIR)/zlib
$(BIN_DIR)/zlib: main.c adapters/zlib_adapter.h
	$(CC) $(CFLAGS) -Dzlib -o $@ main.c $(ZLIB_LIB)

WK64: $(BIN_DIR)/WK64
$(BIN_DIR)/WK64: main.c adapters/WK64_adapter.h
	$(CC) $(CFLAGS) -DWK64 -o $@ main.c $(WK64_LIB)

lzo: $(BIN_DIR)/lzo
$(BIN_DIR)/lzo: main.c adapters/lzo_adapter.h
	$(CC) $(CFLAGS) -Dlzo -o $@ main.c $(LZO_LIB)

zstd: $(BIN_DIR)/zstd
$(BIN_DIR)/zstd: main.c adapters/zstd_adapter.h
	$(CC) $(CFLAGS) -Dzstd -o $@ main.c $(ZSTD_LIB)
	
clean:
	rm -rf $(BIN_DIR)