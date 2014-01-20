
include config.mk

SRC_DIR = $(CURDIR)/src
INC_DIR = $(CURDIR)/include
LIB_DIR = $(CURDIR)/lib

SRC = \
	$(SRC_DIR)/rtld_helpers.c \
	$(SRC_DIR)/rtld_fixup.c \
	$(SRC_DIR)/rtld.c \
	$(SRC_DIR)/list.c \
	$(SRC_DIR)/bintree.c

OBJ = $(SRC:.c=.o)

STATIC_CFLAGS = -static -I$(INC_DIR)
DYNAMIC_CFLAGS = -fpic -I$(INC_DIR)

all: librtld librtld-static test

.PHONY: options
options:
	@echo "librtld build options:"
	@echo " * CC     = $(CC)"
	@echo " * CFLAGS = $(CFLAGS)"

$(LIB_DIR):
	@[ -e $(LIB_DIR) ] || mkdir -p $(LIB_DIR)

.PHONY: librtld
librtld: CFLAGS = $(RTLD_CFLAGS) $(DYNAMIC_CFLAGS)
librtld: options $(OBJ) $(LIB_DIR)
	@echo "=== BUILDING $(RTLD_LIB_DYNAMIC) ==="
	@echo $(CC) -o $(RTLD_LIB_DYNAMIC)
	@$(CC) -o $(RTLD_LIB_DYNAMIC) $(OBJ) $(RTLD_LDFLAGS)
	@mv $(RTLD_LIB_DYNAMIC) $(LIB_DIR)

.PHONY: librtld-static
librtld-static: CFLAGS = $(RTLD_CFLAGS) $(STATIC_CFLAGS)
librtld-static: options $(OBJ) $(LIB_DIR)
	@echo "=== BUILDING $(RTLD_LIB_STATIC) ==="
	@echo ar rcs $(RTLD_LIB_STATIC)
	@ar rcs $(RTLD_LIB_STATIC) $(OBJ)
	@mv $(RTLD_LIB_STATIC) $(LIB_DIR)

.PHONY: test
test:
	@echo "=== BUILDING TEST OBJECTS ==="
	@$(MAKE) -C $(CURDIR)/test $(DEBUG_OPT)

.PHONY: clean
clean:
	@echo "=== CLEANING ==="
	@rm -f $(OBJ)
	@rm -rf $(LIB_DIR)
	@$(MAKE) -C $(CURDIR)/test clean
