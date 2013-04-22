LIB_FILES = lib.c
SRC_FILES = test_loader.c
LIB_OBJECTS = lib.o
OBJECTS = test_loader.o

BINARY = test_loader
LIB_NAME = lib.so
INCLUDE = ../../include

LIB = lib_loader

CFLAGS = -std=gnu99 -Wall -I${INCLUDE}
LDFLAGS = -L ../../lib -l${LIB}

LIB_LDFLAGS = -shared -Wl,-soname,${LIB_NAME}
LIB_CFLAGS = -std=gnu99 -Wall -fpic

ifdef DEBUG
CFLAGS += -g -DDEBUG
LIB_CFLAGS += -g -DDEBUG
endif

all: lib test_loader

lib:
	@echo CC -c ${LIB_FILES}
	@${CC} ${LIB_CFLAGS} -c ${LIB_FILES}
	@echo cc -o ${LIB_NAME}
	@${CC} ${LIB_LDFLAGS} -o ${LIB_NAME} ${LIB_OBJECTS}

test_loader:
	@echo CC -c ${SRC_FILES}
	@${CC} ${CFLAGS} -c ${SRC_FILES}
	@echo CC -o ${BINARY}
	@${CC} -o ${BINARY} ${OBJECTS} ${LDFLAGS}

clean:
	@rm ${OBJECTS} ${LIB_OBJECTS}
	@rm ${LIB_NAME} ${BINARY}

.PHONY: all lib test_loader clean
