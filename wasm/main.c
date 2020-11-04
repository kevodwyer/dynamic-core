#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <emscripten.h>

    char HALT = 0;
    char JUMP = 1;
    char BNE = 2;
    char LOAD_SP = 3;
    char LOAD_EAX_MEM = 4;
    char LOAD_EAX_IB = 5;
    char STORE_EAX = 6;
    char LOAD_EBX_MEM = 7;
    char LOAD_EBX_IB = 8;
    char STORE_EBX = 9;
    char STORE_1 = 10;
    char INC = 11;
    char ADD = 12;
    char MUL = 13;
    char DIV = 14;
    char XOR = 15;
    const char b = 64;

	//char ip = 0;
	//char eax =0;
	//char ebx =0;
	//char sp = b;
	
	char registers[4] = {0,0,0,0};
	int IP = 0;
	int EAX = 1;
	int EBX = 2;
	int SP = 3;	
	int opCounter =0;
	
    char mem[128];
	bool allowedOpcodes[16] = {false,false,false,false,true,false,true,true,false,true,true,true,true,true,true,true}; 


void arraycopy(char *src, int fromIndex, char *dest, int destIndex, int length) {
	for(int i=0;i<length;i++){
		dest[destIndex++]=src[fromIndex++];
	}
}

typedef void (*voidfunc)();
    
const int MAX_FUNCTIONS = 10;
voidfunc func_ptrs[MAX_FUNCTIONS];
int indexToHash[MAX_FUNCTIONS];
int indexToFreq[MAX_FUNCTIONS];

    int module_length = 49; //index 217
    int module_length_2 = 34; //index 227
    int instructionCount = 3; //index 229

    struct Scratch {
        int count;
        char buf[1000];
    };
    
    struct Scratch output;
    struct Scratch tmp;
    struct Scratch leb128;

    void resetScratchpad (struct Scratch *scratch) {
        scratch->count = 0;
    }

    void writeToScratchpad (char b, struct Scratch *scratch) {
        //if (scratch.buf.length == scratch.count) {
        //    System.out.println("array index out of bounds");
        //}
        scratch->buf[scratch->count++] = b;
    }

    void writeBytesToScratchpad(char *b, int off, int len, struct Scratch *scratch) {
        //if ((off < 0) || (off > b.length) || (len < 0) ||
        //        ((off + len) - b.length > 0) || scratch.count + len > scratch.buf.length) {
        //   	printf("array index out of bounds");
        //}
        arraycopy(b, off, scratch->buf, scratch->count, len);
        scratch->count += len;
    }
    
    void writeAllBytesToScratchpad(char *b, int len, struct Scratch *scratch) {
        writeBytesToScratchpad(b, 0, len, scratch);
    }
    
	void writeUnsignedLeb128(int value) {
        resetScratchpad(&leb128);
        int remaining = value >> 7;
        while (remaining != 0) {
            writeToScratchpad(((value & 0x7f) | 0x80), &leb128);
            value = remaining;
            remaining >>= 7;
        }
        writeToScratchpad((value & 0x7f), &leb128);
    }
    void replaceValue(int index, int value) {
        resetScratchpad(&tmp);
        writeBytesToScratchpad(output.buf, 0, index, &tmp);
        writeUnsignedLeb128(value);
        writeBytesToScratchpad(leb128.buf, 0, leb128.count, &tmp);
        writeBytesToScratchpad(output.buf, index +1, output.count - index -1, &tmp);
        resetScratchpad(&output);
        writeBytesToScratchpad(tmp.buf, 0, tmp.count, &output);
    }
    void append(char *instructions, int len, int increment) {
        if(instructionCount == 3) {
            instructionCount = increment;
            writeAllBytesToScratchpad(instructions, len, &output);
        } else {
            writeBytesToScratchpad(instructions, 3, len -3, &output);
            char end[3] = {58,0,0};
            writeAllBytesToScratchpad(end, 3, &output);
            instructionCount = instructionCount + (increment - 4);
        }
        module_length += len;
        module_length_2 += len;
    }
void init() {	

	const int loopLength = 14;
	const int jumpLength = 10;
    char loop[loopLength] = {LOAD_SP, b, STORE_1, LOAD_SP, b,
        	LOAD_EBX_MEM, LOAD_SP, (b+1), LOAD_EAX_MEM, XOR, DIV , LOAD_EBX_MEM, ADD, STORE_EAX};
	char jump[10] = {LOAD_SP, b, LOAD_EAX_MEM, INC, STORE_EAX, LOAD_SP, b+2,
                LOAD_EBX_MEM, BNE, 3};
    char jump2[10] = {LOAD_SP, b+3, LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, b+4,
                LOAD_EBX_MEM, BNE, 0};
    char jump3[10] = {LOAD_SP, b+5, LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, b+6,
                LOAD_EBX_MEM, BNE, 0};
    char jump4[10] = {LOAD_SP, b+7, LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, b+8,
                LOAD_EBX_MEM, BNE, 0};               
	
//	let stack = [1, 0, 11, 1, 11, 1, 11, 1, 11];
	const int stackLength = 9;
	//char stack[stackLength] = {1, 0, 5, 1, 5, 1, 5, 1, 2};
	//char stack[stackLength] = {1, 0, 11, 1, 11, 1, 11, 1, 11};
	char stack[stackLength] = {1, 0, 11, 1, 101, 1, 101, 1, 101};
    arraycopy(loop, 0, mem, 0, 14);
    arraycopy(jump, 0, mem, loopLength , jumpLength);
    arraycopy(jump2, 0, mem, loopLength + jumpLength , jumpLength);
    arraycopy(jump3, 0, mem, loopLength + 2 * jumpLength , jumpLength);
    arraycopy(jump4, 0, mem, loopLength + 3 * jumpLength , jumpLength);
    arraycopy(stack , 0, mem, b, stackLength);
    
    for(int i=0; i < MAX_FUNCTIONS; i++) {
	    indexToHash[i] = -1;
    }
}




	void incrementFrequency(int index) {
		indexToFreq[index] = indexToFreq[index] + 1; 
    }
	int getFrequency(int index) {
		return indexToFreq[index]; 
    }

	int setIndex(int hash) {
    	for(int i=0;i< MAX_FUNCTIONS;i++) {
    		if(indexToHash[i] == -1) {
	    		indexToHash[i] = hash;
    			return i;
    		}
    	}
    	return -1;
    }
	int getIndex(int hash) {
    	for(int i=0;i< MAX_FUNCTIONS;i++) {
    		if(indexToHash[i] == hash) {
    			return i;
    		}
    	}
    	return setIndex(hash);
    }

	int generateHash(char *ops, int opCodesLength) {
    	int hash = 37;
    	for(int i=0;i< opCodesLength;i++) {
    		hash = hash * 37 + ops[i];
    	}
    	return hash;
    }
    bool decodeOp(char op) {
    	opCounter++;
        if(op == HALT) {
            return false;
        }
        else if(op == JUMP) {
            registers[IP] = mem[registers[IP] + 1];
        }
        else if(op == BNE) {
            if (registers[EAX] != registers[EBX])
                registers[IP] = mem[registers[IP] + 1];
            else
                registers[IP] += 2;
        }
        else if(op == LOAD_SP) {
                registers[SP] = mem[registers[IP] + 1];
                registers[IP] += 2;
        }
        else if(op == LOAD_EAX_MEM) {
                registers[EAX] = mem[registers[SP]];
                registers[IP]++;
        }
        else if(op == LOAD_EAX_IB) {
                registers[EAX] = mem[registers[IP] + 1];
                registers[IP] += 2;
        }
        else if(op == STORE_EAX) {
                mem[registers[SP]] = registers[EAX];
                registers[IP]++;
        }
        else if(op == LOAD_EBX_MEM) {
                registers[EBX] = mem[registers[SP]];
                registers[IP]++;
        }
        else if(op == LOAD_EBX_IB) {
                registers[EBX] = mem[registers[IP] + 1];
                registers[IP] += 2;
        }
        else if(op == STORE_EBX) {
                mem[registers[SP]] = registers[EBX];
                registers[IP]++;
        }
        else if(op == STORE_1) {
                mem[registers[SP]] = 1;
                registers[IP]++;
        }
        else if(op == INC) {
                registers[EAX]++;
                registers[IP]++;
        }
        else if(op == ADD) {
                registers[EAX] += registers[EBX];
                registers[IP]++;
        }
        else if(op == MUL) {
                registers[EAX] *= registers[EBX];
                registers[IP]++;
        }
        else if(op == DIV) {
                registers[EAX] /= registers[EBX];
            	registers[IP]++;
         }
        else if(op == XOR) {
                registers[EAX] ^= registers[EBX];
                registers[IP]++;
        }
        else {
             printf("Illegal opcode: %d", op);
            return false;
        }
        return true;
	}
	bool decodeOps(char *ops, int opCodesLength) {
        for(int i=0; i < opCodesLength; i++) {
        	if(!decodeOp(ops[i])){
        		return false;
        	}
        }
        return true;
    }
   
void load(int hash, int index, char *arr, int size) {
	char filename[12];
	sprintf(filename, "%d", hash);
    EM_ASM({
    	try{
    		let size = $1;
    		let buf = new Int8Array(size);
    		for(var i=0;i<size;i++){
    			buf[i] = HEAP8[($0+i)];
    		}	
        	FS.createDataFile("/", $2, buf, true, true);
    	}catch(e){
    		console.log("ex=" + e);
    	}
    }, arr, size, hash);
    void *lib_handle = dlopen(filename, 0);
    voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
    func_ptrs[index] = x;
}

bool blockHack = true;

void compile(int hash, int index, char *opCodes, int opCodesLength) {
    printf("\nhello from compile hash=%d", hash);
    
    //reset
    module_length = 49;
    module_length_2 = 34;
    instructionCount = 3;
    resetScratchpad(&output);
    	
        char header[260] = {0,97,115,109,1,0,0,0,0,12,6,100,121,108,105,110,107,0,0,0,0,0,1,13,3,96,0,0,96,2,127,127,0,96,0,1,127,2,86,5,3,101,110,118,9,115,116,97,99,107,83,97,118,101,0,2,3,101,110,118,13,95,95,109,101,109,111,114,121,95,98,97,115,101,3,127,0,3,101,110,118,12,95,95,116,97,98,108,101,95,98,97,115,101,3,127,0,3,101,110,118,6,109,101,109,111,114,121,2,0,0,3,101,110,118,5,116,97,98,108,101,1,112,0,0,3,5,4,0,0,1,0,6,6,1,127,0,65,0,11,7,74,4,19,95,95,119,97,115,109,95,97,112,112,108,121,95,114,101,108,111,99,115,0,2,12,108,105,98,114,97,114,121,95,102,117,110,99,0,3,12,95,95,100,115,111,95,104,97,110,100,108,101,3,2,18,95,95,112,111,115,116,95,105,110,115,116,97,110,116,105,97,116,101,0,4,10,49,4,4,0,16,2,11,2,0,11,34,1,3,127,16,0,33,2,65,16,33,3,32,2,32,3,107,33,4,32,4,32,0,54,2,12,32,4,32,1,54,2,8};
        writeAllBytesToScratchpad(header, 260, &output);
        
        for(int i=0; i < opCodesLength;i++) {
    		char op = opCodes[i];
	    	if (op ==LOAD_EAX_MEM) {
		        	//cpu.eax = mem[cpu.sp];
        			//cpu.ip++;
        			//to
        			//registers[1] = mem[registers[3]];
    				//registers[0]++;
    				//to
    				char part1[74] = {32,4,40,2,8,33,5,32,4,40,2,12,33,6,32,6,44,0,3,33,7,32,5,32,7,106,33,8,32,8,45,0,0,33,9,32,6,32,9,58,0,1,32,4,40,2,12,33,10,32,10,45,0,0,33,11,65,1,33,12,32,11,32,12,106,33,13,32,10,32,13, 58,0,0};
					append(part1, 74, 12);
		    } else if(op == STORE_EAX) {
		        	//mem[cpu.sp] = cpu.eax;
        			//cpu.ip++;
        			//to
        			//mem[registers[3]] = registers[1];
    				//registers[0]++;
    				if(blockHack) {
    					//block1
	    				char part2[70] = {58,0,0,32,4,40,2,12,33,20,32,20,45,0,1,33,21,32,4,40,2,8,33,22,32,20,44,0,3,33,23,32,22,32,23,106,33,24,32,24,32,21,58,0,0,32,4,40,2,12,33,25,32,25,45,0,0,33,26,32,26,32,12,106,33,27,32,25,32,27};
    	    			append(part2, 70, 12);
    	    			blockHack=false;
        			} else {
        				//block2
        				char part2[70] = {58,0,0,32,4,40,2,12,33,43,32,43,45,0,1,33,44,32,4,40,2,8,33,45,32,43,44,0,3,33,46,32,45,32,46,106,33,47,32,47,32,44,58,0,0,32,4,40,2,12,33,48,32,48,45,0,0,33,49,32,49,32,12,106,33,50,32,48,32,50};
			        	append(part2, 70, 12);
			        }
		    } else if(op == LOAD_EBX_MEM) {
		        	//cpu.ebx = mem[cpu.sp];
        			//cpu.ip++;
        			//to
        			//registers[2] = mem[registers[3]];
            		//registers[0]++;
        			char part3[70] = {58,0,0,32,4,40,2,8,33,28,32,4,40,2,12,33,29,32,29,44,0,3,33,30,32,28,32,30,106,33,31,32,31,45,0,0,33,32,32,29,32,32,58,0,2,32,4,40,2,12,33,33,32,33,45,0,0,33,34,32,34,32,12,106,33,35,32,33,32,35};
			        append(part3, 70, 12);
		    } else if(op == STORE_EBX) {
		        	//mem[cpu.sp] = cpu.ebx;
        			//cpu.ip++;
		    } else if(op == STORE_1) {
		        	//mem[cpu.sp] = 1;
        			//cpu.ip++;
		    } else if(op == INC) {
		        	//cpu.eax++;";
        			//cpu.ip++;";
        			//to
        			//registers[1]++;
    				//registers[0]++;
    				char part4[56] = {58,0,0,32,4,40,2,12,33,14,32,14,45,0,1,33,15,32,15,32,12,106,33,16,32,14,32,16,58,0,1,32,4,40,2,12,33,17,32,17,45,0,0,33,18,32,18,32,12,106,33,19,32,17,32,19};
        			append(part4, 56, 10);
		    } else if(op == ADD) {
		        	//cpu.eax += cpu.ebx;
        			//cpu.ip++; 
        			//td
		            //registers[1] += registers[2];
            		//registers[0]++;
            		char part5[63] = {58,0,0,32,4,40,2,12,33,36,32,36,45,0,2,33,37,32,36,45,0,1,33,38,32,38,32,37,106,33,39,32,36,32,39,58,0,1,32,4,40,2,12,33,40,32,40,45,0,0,33,41,32,41,32,12,106,33,42,32,40,32,42};
			        append(part5, 63, 11);
		    } else if(op == MUL) {
		        	//cpu.eax *= cpu.ebx;
        			//cpu.ip++;
		    } else if(op == DIV) {
		        	//cpu.eax /= cpu.ebx;
        			//cpu.ip++;
        			//to
        			//registers[1] /= registers[2];
        			//registers[0]++; 
        			char part6[63] = {58,0,0,32,4,40,2,12,33,21,32,21,44,0,2,33,22,32,21,44,0,1,33,23,32,23,32,22,109,33,24,32,21,32,24,58,0,1,32,4,40,2,12,33,25,32,25,45,0,0,33,26,32,26,32,12,106,33,27,32,25,32,27};
        			append(part6, 63, 11);
		    } else if(op == XOR) {
		        	//cpu.eax ^= cpu.ebx;
        			//cpu.ip++;
        			//to
        			//registers[1] ^= registers[2];
            		//registers[0]++;
        			char part7[63] = {58,0,0,32,4,40,2,12,33,14,32,14,45,0,2,33,15,32,14,45,0,1,33,16,32,16,32,15,115,33,17,32,14,32,17,58,0,1,32,4,40,2,12,33,18,32,18,45,0,0,33,19,32,19,32,12,106,33,20,32,18,32,20};
        			append(part7, 63, 11);
        	} else {
                    printf("not expected! opcode: %d", op);
	    	}
    	}
    	    char trailer[7] = {15,11,4,0,16,1,11};
        	writeAllBytesToScratchpad(trailer, 7, &output);
        
    	    replaceValue(229, instructionCount);
            replaceValue(227, module_length_2);
            //block should have at least 2 instructions, so...
            //not required result = growArray(result,217, initalInstruction ? module_length : module_length +1);
            replaceValue(217, module_length +1);
			load(hash, index, output.buf, output.count);
}   


int main() {
EM_ASM({console.log("start at " + new Date());});
    printf("hello from main");
    init();
    
    
    char opCodes[10] = {0,0,0,0,0,0,0,0,0,0};
	int opCodesLength = 0;
    char offset = registers[IP];
	int compiledCalls = 0;
	while ( true ) {

        char op = mem[offset++]; 
        if(allowedOpcodes[op]) {
        	opCodes[opCodesLength++] = op;
        } else {
        	if(opCodesLength >= 2) {
	        	int hash = generateHash(opCodes, opCodesLength);
	        	int index = getIndex(hash);
		        incrementFrequency(index);
				int freq = getFrequency(index);
	        	if(freq > 100) {
	    			compiledCalls++;  			
		        		voidfunc fun = func_ptrs[index];
		        		fun(registers, mem);
	            		opCodesLength = 0;
        				if(!decodeOp(op)) {
        					break;
    					}        		
		            	offset = registers[IP];
						continue;
	    		} else if(freq == 100) {
	    			//printf("\nhash=%d index=%d", hash, index);
	    			//for(int j=0;j<opCodesLength;j++){
	    			//	printf("\n[%d %d]", j, opCodes[j]);
	    			//}
	            	compile(hash, index, opCodes, opCodesLength);
	    		}
    		}
        	if(!decodeOps(opCodes, opCodesLength)) {
        		break;
        	}
        	opCodesLength = 0;
        	if(!decodeOp(op)) {
        		break;
    		}        		
            offset = registers[IP];
        }    	
    }
    printf("\nopCounter= %d compiled calls=%d", opCounter, compiledCalls);
    printf("\ndone");
    EM_ASM({console.log("finish at " + new Date());});
}

