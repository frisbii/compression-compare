CC = gcc
CFLAGS = -lm
BIN	= ./bin/


DEPS_DIR = ./deps
# WKdm
WKdm_DIR = $(DEPS_DIR)/WKdm

INCLUDES = -I$(DEPS_DIR)

.PHONY : WKdm lz4

WKdm : $(BIN)/WKdm
$(BIN)/WKdm : main.c WKdm_adapter.h
	$(CC) $(CFLAGS) -DWKdm -o $@ main.c deps/WKdm/WKdm.o  

lz4 : $(BIN)/lz4
$(BIN)/lz4 : main.c lz4_adapter.h
	$(CC) $(CFLAGS) -Dlz4 -o $@ main.c deps/lz4/lib/liblz4.a  

zlib : $(BIN)/zlib
$(BIN)/zlib : main.c zlib_adapter.h
	$(CC) $(CFLAGS) -Dzlib -o $@ main.c deps/zlib-1.3.2/libz.a