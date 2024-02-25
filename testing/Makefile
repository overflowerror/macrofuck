CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs


OBJS     = obj/testing.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: testing.a test

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

testing.a: CFLAGS += -fPIC
testing.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

test: testing.a
	@echo "no tests for testing module"

FORCE: ;

clean:
	@echo "Cleaning up"
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm testing.a
