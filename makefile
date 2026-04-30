CC = gcc
CFLAGS = -Wall -O0 -Iinclude -MMD -MP

SRC_DIR = src
BUILD_DIR = build
TARGET = arc

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

C_SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

LDFLAGS =
LDLIBS = -lm

.PHONY: all install uninstall clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	@echo "Installing binary to $(BINDIR)"
	install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(OBJ:.o=.d)
