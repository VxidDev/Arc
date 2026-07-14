CC = gcc

SRC_DIR = src
BUILD_DIR = build
TARGET = arc

PREFIX ?= /usr
BINDIR = $(DESTDIR)$(PREFIX)/bin

ifneq (,$(findstring mingw, $(CC)))
	ARC_LIB_DIR = "C:/ProgramData/arc/lib"
else
	ARC_LIB_DIR = $(PREFIX)/share/arc/lib
endif 

CLIB_SRC_DIR = stdlib/clib

C_SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

LDLIBS = -lm -lffi

BASE_FLAGS = -Wall -Wextra -Iinclude -DARC_LIB_DIR=\"$(ARC_LIB_DIR)\" -MMD -MP

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

CFLAGS_PROFILE = $(BASE_FLAGS) -O2 -g -pg
LDFLAGS_PROFILE = -pg 

ifneq (,$(findstring mingw,$(CC)))
	LDFLAGS_DEV += -Wl,--export-all-symbols
  LDFLAGS_DEBUG += -Wl,--export-all-symbols
	LDFLAGS_PROFILE += -Wl,--export-all-symbols
	LDFLAGS_RELEASE += -Wl,--export-all-symbols
else
	LDFLAGS_DEV += -rdynamic
  LDFLAGS_DEBUG += -rdynamic
	LDFLAGS_PROFILE += -rdynamic
	LDFLAGS_RELEASE += -rdynamic
endif

CLIB_MODULES = json net
CLIB_TARGETS = $(foreach mod,$(CLIB_MODULES),$(CLIB_SRC_DIR)/$(mod)/build/libarc$(mod).so)

TEST_FILES := $(wildcard tests/*.arc)

V ?= 0

ifeq ($(V),1)
  Q =
else
  Q = @
endif

ECHO = @echo
INSTALL_LIB_DIR = $(DESTDIR)$(ARC_LIB_DIR)

.PHONY: all dev debug release install dev-install debug-install release-install uninstall clean test profile

EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=

all: dev

dev: CFLAGS = $(CFLAGS_DEV) $(EXTRA_CFLAGS)
dev: LDFLAGS = $(LDFLAGS_DEV) $(EXTRA_LDFLAGS)
dev: $(TARGET)

profile: CFLAGS = $(CFLAGS_PROFILE) $(EXTRA_CFLAGS)
profile: LDFLAGS = $(LDFLAGS_PROFILE) $(EXTRA_LDFLAGS)
profile: $(TARGET)

debug: CFLAGS = $(CFLAGS_DEBUG) $(EXTRA_CFLAGS)
debug: LDFLAGS = $(LDFLAGS_DEBUG) $(EXTRA_LDFLAGS)
debug: $(TARGET)

release: CFLAGS = $(CFLAGS_RELEASE) $(EXTRA_CFLAGS)
release: LDFLAGS = $(LDFLAGS_RELEASE) $(EXTRA_LDFLAGS)
release: $(TARGET)

$(CLIB_SRC_DIR)/%/build/libarc%.so:
	$(Q)$(MAKE) -C $(CLIB_SRC_DIR)/$*

release-libs: CFLAGS = $(CFLAGS_RELEASE)
release-libs: $(CLIB_TARGETS)

debug-libs: CFLAGS = $(CFLAGS_DEBUG)
debug-libs: $(CLIB_TARGETS)

dev-libs: CFLAGS = $(CFLAGS_DEV)
dev-libs: $(CLIB_TARGETS)

install-libs:
	$(ECHO) "Installing Arc standard library..."
	$(Q)install -d $(INSTALL_LIB_DIR)/clib
	
	$(MAKE) release-libs
	
	$(Q)cp stdlib/clib/net/axionetd/*.so $(INSTALL_LIB_DIR)/clib/ 2>/dev/null || true
	$(Q)cp stdlib/clib/ui/build/*.so $(INSTALL_LIB_DIR)/clib/ 2>/dev/null || true
	$(Q)cp stdlib/clib/mixer/build/*.so $(INSTALL_LIB_DIR)/clib/ 2>/dev/null || true
	$(Q)cp stdlib/clib/json/build/*.so $(INSTALL_LIB_DIR)/clib/ 2>/dev/null || true
	$(Q)cp stdlib/clib/image/build/*.so $(INSTALL_LIB_DIR)/clib/ 2>/dev/null || true
	
	$(Q)install -d $(INSTALL_LIB_DIR)/include/axionetd
	$(Q)cp -r stdlib/clib/net/axionetd/include/* $(INSTALL_LIB_DIR)/include/axionetd/
	
	$(Q)find stdlib -name "*.arc" -exec install -Dm644 {} $(INSTALL_LIB_DIR)/{} \;

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
	$(Q)$(MAKE) install-libs

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
