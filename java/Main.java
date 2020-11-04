

public class Main {

    public Main() {
        long start = System.currentTimeMillis();
        run();
        long end = System.currentTimeMillis();
        long duration = end - start;
        long seconds = duration / 1000;
        System.out.println("done elapsed seconds:" + seconds);
    }


    final byte HALT = 0;
    final byte JUMP = 1;
    final byte BNE = 2;
    final byte LOAD_SP = 3;
    final byte LOAD_EAX_MEM = 4;
    final byte LOAD_EAX_IB = 5;
    final byte STORE_EAX = 6;
    final byte LOAD_EBX_MEM = 7;
    final byte LOAD_EBX_IB = 8;
    final byte STORE_EBX = 9;
    final byte STORE_1 = 10;
    final byte INC = 11;
    final byte ADD = 12;
    final byte MUL = 13;
    final byte DIV = 14;
    final byte XOR = 15;

    private void run() {

        //first 64 bytes are code, second 64 bytes are stack/data
        byte[] mem = new byte[128];

        // write code
        byte b = 64;
        byte [] loop = new byte [] {LOAD_SP, b, STORE_1, LOAD_SP, b,
                LOAD_EBX_MEM, LOAD_SP, (byte)(b+1), LOAD_EAX_MEM, XOR, DIV , LOAD_EBX_MEM, ADD, STORE_EAX } ;
        byte[] jump = new byte[] {LOAD_SP, b, LOAD_EAX_MEM, INC, STORE_EAX, LOAD_SP, (byte)(b+2),
                LOAD_EBX_MEM, BNE, 3};
        byte[] jump2 = new byte[] {LOAD_SP, (byte)(b+3), LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, (byte)(b+4),
                LOAD_EBX_MEM, BNE, 0};
        byte[] jump3 = new byte[] {LOAD_SP, (byte)(b+5), LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, (byte)(b+6),
                LOAD_EBX_MEM, BNE, 0};
        byte[] jump4 = new byte[] {LOAD_SP, (byte)(b+7), LOAD_EAX_MEM, INC,
                STORE_EAX, LOAD_SP, (byte)(b+8),
                LOAD_EBX_MEM, BNE, 0};
        //byte [] stack = new byte [] {1, 0, 101, 1, 101, 1, 101, 1, 101};
        byte [] stack = new byte [] {1, 0, 101, 1, 101, 1, 101, 1, 101};
        System.arraycopy(loop, 0, mem, 0, loop.length);
        System.arraycopy(jump, 0, mem, loop.length , jump.length);
        System.arraycopy(jump2, 0, mem, loop.length + jump.length , jump2.length);
        System.arraycopy(jump3, 0, mem, loop.length + 2 * jump.length , jump3.length);


        System.arraycopy(jump4, 0, mem, loop.length + 3 * jump.length , jump4.length);
        System.arraycopy(stack , 0, mem, b, stack.length);
        byte ip = 0, eax=0, ebx=0, sp = b;
        cpu :
        while ( true ) {
            //System.out.println("opcode: " + mem[ip] + " at ip " + ip);
            switch (mem[ip]) {
                case HALT:
                    break cpu;
                case JUMP:
                    ip = mem[ip + 1];
                    break;
                case BNE:
                    if (eax != ebx)
                        ip = mem[ip + 1];
                    else
                        ip += 2;
                    break;
                case LOAD_SP:
                    sp = mem[ip + 1];
                    ip += 2;
                    break;
                case LOAD_EAX_MEM:
                    eax = mem[sp];
                    ip++;
                    break;
                case LOAD_EAX_IB:
                    eax = mem[ip + 1];
                    ip += 2;
                    break;
                case STORE_EAX:
                    mem[sp] = eax;
                    ip++;
                    break;
                case LOAD_EBX_MEM:
                    ebx = mem[sp];
                    ip++;
                    break;
                case LOAD_EBX_IB:
                    ebx = mem[ip + 1];
                    ip += 2;
                    break;
                case STORE_EBX:
                    mem[sp] = ebx;
                    ip++;
                    break;
                case STORE_1:
                    mem[sp] = 1;
                    ip++;
                    break;
                case INC:
                    eax++;
                    ip++;
                    break;
                case ADD:
                    eax += ebx;
                    ip++;
                    break;
                case MUL:
                    eax *= ebx;
                    ip++;
                    break;
                case DIV:
                    eax /= ebx;
                    ip++;
                    break;
                case XOR:
                    eax ^= ebx;
                    ip++;
                    break;
                default:
                    System.out.println("Illegal opcode: " + mem[ip] + " at ip " + ip);
            }
        }
        System.out.println("outside");
    }
    public static void main(String[] args) {
        new Main();
    }
}
