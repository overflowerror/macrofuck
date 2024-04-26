CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g -I./src/ -I../common/src/ -I./gen
LD       = gcc
LDFLAGS  = 
YACC     = bison
YFLAGS   = -d
LEX      = lex
LEXFLAGS =
MAKE     = make


PLUGINS  =
BUILTINS = obj/builtins/builtins.o obj/builtins/numbers.o \
           obj/builtins/print.o
OBJS     = obj/lex.yy.o obj/y.tab.o obj/codegen.o obj/ast.o obj/band.o \
           obj/plugins.o obj/scope.o $(PLUGINS) $(BUILTINS) obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: macrofuck test

gen/lex.yy.c: src/lexer.l gen/y.tab.c
	$(LEX) $(LEXFLAGS) -o $@ $<

gen/y.tab.c: src/parser.y src/ast.h ../common/src/error.h
	$(YACC) $(YFLAGS) -o $@ $<

obj/lex.yy.o: gen/lex.yy.c
	$(CC) $(CFLAGS) -c -o $@ $<
obj/y.tab.o: gen/y.tab.c
	$(CC) $(CFLAGS) -c -o $@ $<
obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

macrofuck: $(OBJS) ../common/common.a
	$(LD) $(LDFLAGS) -o $@ $^

../common/common.a: FORCE
	cd ../common/ && $(MAKE) common.a

test: macrofuck FORCE
	cd ../common/ && $(MAKE) test
	./test/tests.sh "./macrofuck"

FORCE: ;

clean:
	@echo "Cleaning up"
	@cd ../common/ && $(MAKE) clean
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f gen/*.c
	@rm -f gen/*.h
	@rm macrofuck
