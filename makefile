CC = gcc

SRC_DIR = src
BUILD_DIR = build
TARGET = arc

PREFIX ?= /usr
BINDIR = $(DESTDIR)$(PREFIX)/bin

C_SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

LDLIBS = -lm

BASE_FLAGS = -Wall -Wextra -Iinclude -MMD -MP

CFLAGS_DEV = $(BASE_FLAGS) -O0 -g

CFLAGS_DEBUG = $(BASE_FLAGS) -O0 -g \
	-fsanitize=address,undefined \
	-fno-omit-frame-pointer

LDFLAGS_DEBUG = -fsanitize=address,undefined

CFLAGS_RELEASE = $(BASE_FLAGS) -O3 -flto -march=native
LDFLAGS_RELEASE = -flto

V ?= 0

ifeq ($(V),1)
  Q =
else
  Q = @
endif

ECHO = @echo

.PHONY: all dev debug release install dev-install debug-install release-install uninstall clean

all: dev

dev: CFLAGS = $(CFLAGS_DEV)
dev: LDFLAGS =
dev: $(TARGET)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: $(TARGET)

release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: $(TARGET)

$(TARGET): $(OBJ)
	$(Q)$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

dev-install: dev
	$(ECHO) "Installing DEV -> $(BINDIR)"
	$(Q)install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

debug-install: debug
	$(ECHO) "Installing DEBUG -> $(BINDIR)"
	$(Q)install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

release-install: release
	$(ECHO) "Installing RELEASE -> $(BINDIR)"
	$(Q)install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

install: release-install

uninstall:
	$(Q)rm -f $(BINDIR)/$(TARGET)

clean:
	$(Q)rm -rf $(BUILD_DIR) $(TARGET)

-include $(OBJ:.o=.d)
