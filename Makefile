all: pong

pong: main.c Logs/log.c Logs/log.h RNG/rng.c RNG/rng.h
	gcc -Wall -Wextra -Werror -lgdiplus -mwindows -o pong.exe main.c Logs/log.c RNG/rng.c
	
.PHONY: all clean

clean:
	rm "pong.exe"