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
CFLAGS_RELEASE = \
  $(BASE_FLAGS) \
  -O3 \
  -flto=auto \
  -march=native \
  -mtune=native \
  -ffunction-sections \
  -fdata-sections \
  -fno-semantic-interposition \
  -pipe \
  -fomit-frame-pointer \
  -fno-math-errno \
  -fno-stack-protector \
  -fno-unwind-tables \
  -fno-asynchronous-unwind-tables \
	-falign-functions=16 \
  -falign-loops=16

LDFLAGS_RELEASE = \
  -flto=auto \
  -Wl,--gc-sections \
  -Wl,-O2

LDFLAGS_RELEASE += -rdynamic
LDFLAGS_DEBUG += -rdynamic

CFLAGS_PROFILE = $(BASE_FLAGS) -O2 -g -pg
LDFLAGS_PROFILE = -pg -rdynamic

TEST_FILES := $(wildcard tests/*.arc)

V ?= 0

ifeq ($(V),1)
  Q =
else
  Q = @
endif

ECHO = @echo

.PHONY: all dev debug release install dev-install debug-install release-install uninstall clean test profile

all: dev

dev: CFLAGS = $(CFLAGS_DEV)
dev: LDFLAGS = -rdynamic
dev: $(TARGET)

profile: CFLAGS = $(CFLAGS_PROFILE)
profile: LDFLAGS = $(LDFLAGS_PROFILE)
profile: $(TARGET)

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

test: release
	$(ECHO) "Running tests..."
	$(Q)for f in $(TEST_FILES); do \
		echo "== $$f =="; \
		./$(TARGET) "$$f" || exit 1; \
	done
	$(ECHO) "All tests passed."

install: release-install

uninstall:
	$(Q)rm -f $(BINDIR)/$(TARGET)

clean:
	$(Q)rm -rf $(BUILD_DIR) $(TARGET)

-include $(OBJ:.o=.d)
