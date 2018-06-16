SRCS = tcp_cli.c
TARGET = TCP_Client
OBJS = ${SRCS:.c=.o}
CC ?= gcc

INC_DIR	?= -I${PWD}
CFLAGS	?= -Wall -O0 -g

all: ${TARGET}

${TARGET} : ${OBJS}
	$(CC) -o $@ $(CFLAGS) $?
.c.o : 
	${CC} -c ${CFLAGS} ${INC_DIR} $<

clean:
	rm -f ${OBJS}
	rm ${TARGET}
