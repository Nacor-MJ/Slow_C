CFLAGS=-std=c11 -g -fno-common -Wall -Wno-switch

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

# Stage 1

Slow_C: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)
	rm *.o

$(OBJS): slow_c.h vec.h

git_pull:
	git pull https://github.com/Nacor-MJ/Slow_C/ master

git_push:
	git push origin master

# Misc.

clean:
	-rm main.exe.stackdump
	-rm -rf slow_c tmp* 
	-rm *.exe

.PHONY: test clean test-stage2
