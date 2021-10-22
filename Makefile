OBJS = src/lex.yy.o src/C.tab.o src/symbol_table.o src/nodes.o src/main.o src/interpreter.o
SRCS = src/lex.yy.c src/C.tab.c src/symbol_table.c src/nodes.c src/main.c src/interpreter.c

#TEST_UNIT_OBJS =
#TEST_UNIT_SRCS =
CC = gcc

all:	mycc #test

#test: mycc


clean:
	rm ${OBJS}

mycc:	${OBJS}
	${CC} -g -o mycc ${OBJS} -I /headers

lex.yy.c: C.flex
	flex C.flex

C.tab.c:	C.y
	bison -d -t -v C.y

.c.o:
	${CC} -g -c $*.c

depend:	
	${CC} -M $(SRCS) > .deps
	cat Makefile .deps > makefile

dist:	symbol_table.c nodes.c main.c Makefile C.flex C.y nodes.h token.h interpreter.c interpreter.h
	tar cvfz mycc.tgz symbol_table.c nodes.c main.c Makefile C.flex C.y \
		nodes.h token.h interpreter.c interpreter.h
