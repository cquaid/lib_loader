LOADER_SRC = libc_loader.c
LOADER_OBJ = $(LOADER_SRC:.c=.o)

BINARY = libc_loader

RTLD_LIB ?= rtld
RTLD_LIB_DIR = $(CURDIR)/../../lib
RTLD_INC_DIR = $(CURDIR)/../../include

LOADER_CFLAGS = -std=gnu99 -Wall -I$(RTLD_INC_DIR)
LOADER_LDFLAGS = -static -L$(RTLD_LIB_DIR) -l$(RTLD_LIB)

ifdef DEBUG
LOADER_CFLAGS += -g -DDEBUG
endif

LD_PATH ?= $(shell ldd $$(which ls) | awk '/ld/{print $$1;}')
LIBC_PATH ?= $(shell ldd $$(which ls) | awk '/libc/{print $$3;}')

all: $(BINARY)

.PHONY: $(BINARY)
$(BINARY): CFLAGS = $(LOADER_CFLAGS) \
	-DLIBC_PATH=\"$(LIBC_PATH)\" \
	-DLD_PATH=\"$(LD_PATH)\"
$(BINARY): $(LOADER_OBJ)
	@echo $(CC) -o $(BINARY)
	@$(CC) -o $(BINARY) $(LOADER_OBJ) $(LOADER_LDFLAGS)

.PHONY: clean
clean:
	@rm -f $(BINARY) $(LOADER_OBJ)
