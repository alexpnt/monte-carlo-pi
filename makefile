FLAGS= -Wall -g -lm
CC = gcc
PROG = montecarlo
OBJS = montecarlo.o



# GENERIC
all:		${PROG}

clean:
		rm ${OBJS} ${PROG} 

${PROG}:	${OBJS}
		${CC} ${FLAGS} ${OBJS} -o $@

#############################

input.o:	montecarlo.c
