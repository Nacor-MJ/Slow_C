CFLAGS=-std=c11 -g3 -fno-common -Wall -Wno-switch -Werror -fms-extensions

SRCS=$(filter-out idk.c, $(wildcard *.c))
OBJS=$(SRCS:.c=.o)

# Stage 1

Slow_C: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS) -g -lm
	rm *.o
	clear

$(OBJS): slow_c.h

gdb: Slow_C
	gdb -ex run --args ./Slow_C.exe idk.c

test_idk: Slow_C
	./Slow_C.exe idk.c
	-./idk.exe

git:
	./git.ps1

valgrind: Slow_C
	valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./Slow_C idk.c

# Misc.

clean:
	-rm main.exe.stackdump
	-rm -rf slow_c tmp* 
	-rm *.exe
