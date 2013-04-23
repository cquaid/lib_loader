LIB_FILES = malloc_test.c
SRC_FILES = malloc_loader.c
LIB_OBJECTS = malloc_test.o
OBJECTS = malloc_loader.o

BINARY = malloc_loader
LIB_NAME = malloc_test.so
INCLUDE = ../../include

LIB = rtld

CFLAGS = -std=gnu99 -Wall -I${INCLUDE}
LDFLAGS = -L ../../lib -l${LIB}

LIB_LDFLAGS = -shared -Wl,-soname,${LIB_NAME}
LIB_CFLAGS = -std=gnu99 -Wall -fpic

ifdef DEBUG
CFLAGS += -g -DDEBUG
LIB_CFLAGS += -g -DDEBUG
endif

all: malloc_test malloc_loader

malloc_test:
	@echo CC -c ${LIB_FILES}
	@${CC} ${LIB_CFLAGS} -c ${LIB_FILES}
	@echo cc -o ${LIB_NAME}
	@${CC} ${LIB_LDFLAGS} -o ${LIB_NAME} ${LIB_OBJECTS}

malloc_loader:
	@echo CC -c ${SRC_FILES}
	@${CC} ${CFLAGS} -c ${SRC_FILES}
	@echo CC -o ${BINARY}
	@${CC} -o ${BINARY} ${OBJECTS} ${LDFLAGS}

clean:
	@rm ${OBJECTS} ${LIB_OBJECTS}
	@rm ${LIB_NAME} ${BINARY}

.PHONY: all malloc_test malloc_loader clean
