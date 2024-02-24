CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g -I./src -I./../common/src/
LD       = gcc
LDFLAGS  = 
YACC     = bison
YFLAGS   = -d
LEX      = lex
LEXFLAGS =


OBJS     = obj/networking.o obj/header.o obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: macrofuck-lsp

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

macrofuck-lsp: $(OBJS) ../common/common.a
	$(LD) $(LDFLAGS) -o $@ $^

../common/common.a: FORCE
	cd ../common/ && $(MAKE) common.a

FORCE: ;

clean:
	@cd ../common/ && $(MAKE) clean
	@echo "Cleaning up"
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f gen/*.c
	@rm -f gen/*.h
	@rm macrofuck-lsp
