#!/bin/bash

set -eux
set -o pipefail

time ./sieve 1000 > /dev/null
time ./sieve 10000 > /dev/null
