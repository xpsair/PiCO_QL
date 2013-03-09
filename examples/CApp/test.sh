#!/bin/sh
make prep
make clean
rm pico_ql_internal.c pico_ql_search.c > /dev/null
ruby pico_ql_generator.rb pico_ql_dsl.sql C
make PICO_QL_JOIN_THREADS=1 
#G_CXX=1
gdb ./capp
