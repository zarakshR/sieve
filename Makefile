all: sieve decode

sieve: sieve.c
	gcc -g -Wall -Wextra -Wpedantic -O3 -D_FORTIFY_SOURCE=2 sieve.c -o sieve

decode: decode.c
	gcc -g -Wall -Wextra -Wpedantic -O3 -D_FORTIFY_SOURCE=2 decode.c -o decode
