CFLAGS=-std=c11 -g3 -fno-common -Wall -Wno-switch

SRCS=$(filter-out idk.c, $(wildcard *.c))
OBJS=$(SRCS:.c=.o)

# Stage 1

Slow_C: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)
	rm *.o

$(OBJS): slow_c.h vec.h

gdb: Slow_C
	gdb ./Slow_C.exe

test_idk: Slow_C
	./Slow_C.exe idk.txt 
	-./idk.exe

git:
	Set-ExecutionPolicy AllSigned
	./git.ps1


# Misc.

clean:
	-rm main.exe.stackdump
	-rm -rf slow_c tmp* 
	-rm *.exe
