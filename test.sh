#!/bin/bash

set -eux
set -o pipefail

if ! make; then exit 1; else :; fi

diff -sq <(./sieve 1000 | ./decode) primes1k.txt
diff -sq <(./sieve 10000 | ./decode) primes10k.txt
