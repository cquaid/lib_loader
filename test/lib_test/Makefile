LIB_SRC = lib.c
LOADER_SRC = test_loader.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LOADER_OBJ = $(LOADER_SRC:.c=.o)

BINARY = test_loader
LIB_NAME = lib.so

RTLD_LIB ?= rtld
RTLD_LIB_DIR = $(CURDIR)/../../lib
RTLD_INC_DIR = $(CURDIR)/../../include

LOADER_CFLAGS = -std=gnu99 -Wall -I$(RTLD_INC_DIR)
LOADER_LDFLAGS = -static -L$(RTLD_LIB_DIR) -l$(RTLD_LIB)

LIB_LDFLAGS = -shared -Wl,-soname,$(LIB_NAME)
LIB_CFLAGS = -std=gnu99 -Wall -fpic

ifdef DEBUG
LOADER_CFLAGS += -g -DDEBUG
LIB_CFLAGS += -g -DDEBUG
endif

all: lib $(BINARY)

.PHONY: lib
lib: CFLAGS = $(LIB_CFLAGS)
lib: $(LIB_OBJ)
	@echo $(CC) -o $(LIB_NAME)
	@$(CC) -o $(LIB_NAME) $(LIB_OBJ) $(LIB_LDFLAGS)

.PHONY: $(BINARY)
$(BINARY): CFLAGS = $(LOADER_CFLAGS)
$(BINARY): lib $(LOADER_OBJ)
	@echo $(CC) -o $(BINARY)
	@$(CC) -o $(BINARY) $(LOADER_OBJ) $(LOADER_LDFLAGS)

.PHONY: clean
clean:
	@rm -f $(LIB_OBJ) $(LOADER_OBJ)
	@rm -f $(LIB_NAME) $(BINARY)
