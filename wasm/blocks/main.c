#include <dlfcn.h>
#include <stdio.h>
#include <emscripten.h>
    typedef void (*voidfunc)();
    
    int registers[4] = {0,1,2,3};
    char mem[128];
        
void next(const char *z) {
    void *lib_handle = dlopen("library.wasm", 0);

    voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
    if (!x) printf("\ncannot find side function");
    else x(registers, mem);
    for(int i=0; i<4;i++){
    	printf("\nreg=%d",registers[i]);
    }
}
int main() {
    printf("\nhello from main");
    emscripten_async_wget("library.wasm", "library.wasm", next, NULL);
    printf("\nasync");
    return 0;
}
