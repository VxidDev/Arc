CC = gcc

CFLAGS = -Wall -O0 -g -Iinclude -fPIC 
LDFLAGS = -Wl,-Bsymbolic-functions

SRC_DIR = src
BUILD_DIR = build
LIB = libaxionetd.so

DESTDIR ?=
PREFIX ?= /usr/local
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/axionetd

C_SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC)) 

all: $(LIB)

$(LIB): $(OBJ)
	$(CC) -shared -o $@ $(OBJ) $(LDFLAGS) -lyyjson

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIB)
	@echo "Installing library to $(LIBDIR)"
	install -d $(LIBDIR)
	install -m 755 $(LIB) $(LIBDIR)/$(LIB)

	@echo "Installing headers to $(INCLUDEDIR)"
	install -d $(INCLUDEDIR)
	cp -r include/* $(INCLUDEDIR)

uninstall:
	rm -f $(LIBDIR)/$(LIB)
	rm -rf $(INCLUDEDIR)
	ldconfig

clean:
	rm -rf $(BUILD_DIR) $(LIB)
