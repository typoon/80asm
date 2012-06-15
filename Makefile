CC=gcc
FLAGS=-Wall -ggdb

PROG=80asm
OBJS=$(shell ls *.c | sed -e "s/\.c/.o/g")
YACC=bison

all: parser $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o bin/$(PROG)

parser:
	bison -d parser.y
	flex  -i parser.l
	$(CC) $(FLAGS) parser.tab.c parser.tab.h -c
	$(CC) $(FLAGS) lex.yy.c -c
	
	$(eval $OBJS+=parser.tab.o lex.yy.o)

%.o: %.c
	$(CC) $(FLAGS) -c $<

clean:
	@rm -f *.o
	@rm -f *.gch
	@rm -f parser.tab.c parser.tab.h lex.yy.c
	@rm -rf bin/*


