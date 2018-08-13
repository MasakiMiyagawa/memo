SRCS = https_server.c
TARGET = https_server
OBJS = ${SRCS:.c=.o}
CC ?= gcc

INC_DIR	?= -I${PWD}
CFLAGS	?= -Wall -O0 -g
LIBS ?= -lssl -lcrypto -lstdc++ -lm

all: ${TARGET}

${TARGET} : ${OBJS}
	$(CC) -o $@ $(CFLAGS) $? $(LIBS)
.c.o : 
	${CC} -c ${CFLAGS} ${INC_DIR} $<

clean:
	rm -f ${OBJS}
	rm ${TARGET}
