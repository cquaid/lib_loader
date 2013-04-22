SRC_DIR = .
SRC_FILES = ${SRC_DIR}/zlib_list.c ${SRC_DIR}/libz_loader.c
OBJECTS = zlib_list.o libz_loader.o

LIB = lib_loader
INCLUDE = ../../include
BINARY = libz_loader

CFLAGS = -std=gnu99 -Wall -I${INCLUDE}
LDFLAGS = -L../../lib -l${LIB}

ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

all: libz_loader

libz_loader:
	@echo CC -c ${SRC_FILES} ${LDFLAGS}
	@${CC} ${CFLAGS} -c ${SRC_FILES}
	@echo CC -o ${BINARY}
	@${CC} -o ${BINARY} ${OBJECTS} ${LDFLAGS}

clean:
	@rm ${OBJECTS}
	@rm ${BINARY}

.PHONY: all libz_loader clean
