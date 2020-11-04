#include <stdio.h> 
	int IP = 0;
	int EAX = 1;
	int EBX = 2;
	int SP = 3;	
void library_func(char *registers, char *mem) {
    registers[EAX] = mem[registers[SP]];
    registers[IP]++;

    registers[EAX]++;
    registers[IP]++;

    mem[registers[SP]] = registers[EAX];
    registers[IP]++;
}
