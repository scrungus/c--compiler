OBJS = src/lex.yy.o src/C.tab.o src/symbol_table.o src/nodes.o src/main.o src/interpreter.o
SRCS = src/lex.yy.c src/C.tab.c src/symbol_table.c src/nodes.c src/main.c src/interpreter.c

TEST_UNIT_OBJS =src/lex.yy.o src/C.tab.o src/symbol_table.o src/nodes.o src/interpreter.o  objs/test_utilities.o
TEST_UNITS = test_compiler test_interpreter
CC = gcc

all:	mycc test

test: mycc
	$(CC) -g -c test/test_utilities.c -I./headers/ -o objs/test_utilities.o
	$(CC) -g test/test_interpreter.c -I./headers/ -o test_interpreter $(TEST_UNIT_OBJS)
	$(CC) -g test/test_compiler.c -I./headers/ -o test_compiler $(TEST_UNIT_OBJS)
	sh test/run_tests.sh

clean:
	rm ${OBJS} ${TEST_UNITS}

mycc:	${OBJS}
	${CC}  -I./headers/ -g -o mycc ${OBJS}

lex.yy.c: C.flex
	flex C.flex

C.tab.c:	C.y
	bison -d -t -v C.y

.c.o:
	${CC} -g -c -I./headers/ $*.c -o $*.o

depend:	
	${CC} -M $(SRCS) > .deps
	cat Makefile .deps > makefile

dist:	symbol_table.c nodes.c main.c Makefile C.flex C.y nodes.h token.h interpreter.c interpreter.h
	tar cvfz mycc.tgz symbol_table.c nodes.c main.c Makefile C.flex C.y \
		nodes.h token.h interpreter.c interpreter.h
