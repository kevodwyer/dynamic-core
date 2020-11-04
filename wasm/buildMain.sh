#/bin/sh
emcc main.c -s MAIN_MODULE=2  -s ALLOW_MEMORY_GROWTH=1 -O1 -o exe.html -s EXPORTED_FUNCTIONS='["_main","_puts"]'
