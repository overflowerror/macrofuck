CC       = gcc
CFLAGS   = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wpedantic -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs


OBJS     = obj/main.o
DEPS     = $(OBJS:%.o=%.d)

-include $(DEPS)

all: bf test

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c -o $@ $<


bf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^


test: bf
	./test/tests.sh "./bf"


FORCE: ;

clean:
	@echo "Cleaning up"
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm bf
