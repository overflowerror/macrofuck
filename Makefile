CC       = gcc
CFLAGS   = -std=c11 -Wall -Wpedantic -g -I./src -I./gen
LD       = gcc
LDFLAGS  = 
YACC     = bison
YFLAGS   = -d
LEX      = lex
LEXFLAGS =


PLUGINS  = obj/macros/numbers.o
OBJS     = obj/lex.yy.o obj/y.tab.o obj/codegen.o obj/error.o \
           obj/ast.o obj/alloc.o obj/dict.o obj/list.o obj/band.o \
           obj/plugins.o $(PLUGINS) obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: macrofuck test

gen/lex.yy.c: src/lexer.l gen/y.tab.c
	$(LEX) $(LEXFLAGS) -o $@ $<

gen/y.tab.c: src/parser.y src/ast.h src/error.h
	$(YACC) $(YFLAGS) -o $@ $<

obj/lex.yy.o: gen/lex.yy.c
	$(CC) $(CFLAGS) -c -o $@ $<
obj/y.tab.o: gen/y.tab.c
	$(CC) $(CFLAGS) -c -o $@ $<
obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

macrofuck: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

test: macrofuck FORCE
	./test/tests.sh "./macrofuck"

FORCE: ;

clean:
	@echo "Cleaning up"
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f gen/*.c
	@rm -f gen/*.h
	@rm macrofuck
