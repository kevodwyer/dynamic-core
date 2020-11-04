#/bin/sh
emcc main.c -s MAIN_MODULE=2 -O0 -o exe.html -s EXPORTED_FUNCTIONS='["_main","_puts"]'
