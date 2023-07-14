sieve: sieve.c
	gcc -g -Wall -Wextra -Wpedantic -O3 -D_FORTIFY_SOURCE=2 sieve.c -o sieve
