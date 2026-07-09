CC = gcc

BUILD_DIR = build
TARGET_LIB = libarcmixer.so

BASE_FLAGS = -Wall -Wextra -I../../../include -MMD -MP -fPIC -shared -lSDL3 -lSDL3_mixer
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
LDFLAGS_PROFILE = -pg
.PHONY: all dev debug release profile clean

CFLAGS_DEV = $(BASE_FLAGS) -O0 -g
LDFLAGS_DEV = -rdynamic

all: dev

dev: CFLAGS = $(CFLAGS_DEV)
dev: LDFLAGS = $(LDFLAGS_DEV)
dev: $(TARGET_LIB)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: $(TARGET_LIB)

release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: $(TARGET_LIB)

profile: CFLAGS = $(CFLAGS_PROFILE)
profile: LDFLAGS = $(LDFLAGS_PROFILE)
profile: $(TARGET_LIB)

$(TARGET_LIB): mixer.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) mixer.c -o $(BUILD_DIR)/$@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
