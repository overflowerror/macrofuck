CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g -I./src -I./gen
LD       = gcc
LDFLAGS  = 
YACC     = bison
YFLAGS   = -d
LEX      = lex
LEXFLAGS =


OBJS     = obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: macrofuck-lsp

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

macrofuck-lsp: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

FORCE: ;

clean:
	@echo "Cleaning up"
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f gen/*.c
	@rm -f gen/*.h
	@rm macrofuck-lsp
