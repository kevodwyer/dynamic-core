#/bin/sh
emcc library.c -s SIDE_MODULE=1 -O0 -gseparate-dwarf=whatever.debug.wasm -g0 -o library.wasm
