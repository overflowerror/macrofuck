CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g -I./src -I./gen
LD       = gcc
LDFLAGS  = 
YACC     = bison
YFLAGS   = -d
LEX      = lex
LEXFLAGS =


PLUGINS  = obj/macros/numbers.o
OBJS     = obj/lex.yy.o obj/y.tab.o obj/codegen.o obj/error.o \
           obj/ast.o obj/alloc.o obj/dict.o obj/list.o obj/band.o \
           obj/plugins.o obj/scope.o $(PLUGINS) obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: macrofuck test

gen/lex.yy.c: src/lexer.l gen
	$(LEX) $(LEXFLAGS) -o $@ $<

gen/y.tab.c: src/parser.y src src
	$(YACC) $(YFLAGS) -o $@ $<

obj/lex.yy.o: gen
	$(CC) $(CFLAGS) -c -o $@ $<
obj/y.tab.o: gen
	$(CC) $(CFLAGS) -c -o $@ $<
obj/%.o: src obj
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