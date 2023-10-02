#!/bin/bash
# gcc -c lib/present_bits/present.c -g -o present.o
# gcc -c msgpack_format_ver1.c -g -o msgpack_format_ver1.o
# gcc present.o msgpack_format_ver1.o -g -o ./bin/msgpack_format_ver1

gcc -c lib/short_msgpack/s_msgpack.c -g -o s_msgpack.o
gcc -c use_msgpack1.c -g -o use_msgpack1.o

gcc s_msgpack.o use_msgpack1.o -g -o ./bin/use_msgpack1

rm -rf *.o
# gcc msgpack_format.c lib/present_bits/present.c -g -o msgpack_format
# gcc msgpack_format_ver1.c lib/present_bits/present.c -g -o msgpack_format_ver1