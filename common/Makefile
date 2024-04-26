CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs


OBJS     = obj/alloc.o obj/dict.o obj/error.o obj/list.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: common.a test

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

common.a: CFLAGS += -fPIC
common.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

test: CFLAGS += -I../testing/src/
test: obj/tests.o common.a ../testing/testing.a
	cd ../testing/ && $(MAKE) test
	$(LD) $(LDFLAGS) -o $@ $^
	./test

../testing/testing.a: FORCE
	cd ../testing/ && $(MAKE) testing.a

FORCE: ;

clean:
	@echo "Cleaning up"
	cd ../testing/ && $(MAKE) clean
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm common.a
