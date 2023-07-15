#!/bin/bash

set -eux
set -o pipefail

make

./sieve 1000 > nums1k.bin
./decode > nums1k.txt
if $(diff nums1k.txt primes1k.txt); then echo "OK!"; else echo "FAIL!"; fi
