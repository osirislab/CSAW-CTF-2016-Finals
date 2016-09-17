#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>

#include "memory.h"

memory main_memory;

#define INST_PTR_LOC 0x0
#define STK_PTR_LOC  0x1
#define BASE_PTR_LOC 0x2
#define SCRATCH_REG  0x3

#define CODE_START 0x1000

#define INSTR(opcode, value, family, doc) opcode = value,
enum op {
    INSTR(MI, 0x0, ARITH, "Move Immediate")
    INSTR(MV, 0x1, ARITH, "Move Memory")
    INSTR(MD, 0x2, ARITH, "Move Dereference")
    INSTR(LD, 0x3, ARITH, "Load Memory")
    INSTR(ST, 0x4, ARITH, "Store Memory")
    INSTR(AD, 0x5, ARITH, "Add Memory")
    INSTR(SB, 0x6, ARITH, "Subtract Memory")
    INSTR(ND, 0x7, ARITH, "Bitwise And Memory")
    INSTR(OR, 0x8, ARITH, "Bitwise Or Memory")
    INSTR(XR, 0x9, ARITH, "Bitwise Xor Memory")
    INSTR(SR, 0xA, ARITH, "Right Shift")
    INSTR(SL, 0xB, ARITH, "Left Shift")
    INSTR(SA, 0xC, ARITH, "Right Shift (arithmetic)")
    INSTR(JG, 0xD, LOGIC, "Jump if Greater-Than")
    INSTR(JL, 0xE, LOGIC, "Jump if Less-Than")
    INSTR(JQ, 0xF, LOGIC, "Jump if Equal-To")
};

// macro to print instruction traces, + ip
#ifdef DO_TRACE
#  define P_TRACE(fmt, ...) fprintf(stderr, "0x%04x: " fmt "\n", ip, __VA_ARGS__)
#else
#  define P_TRACE(fmt, ...)
#endif

#define OPCODE(addr) (op((main_memory[addr] & 0xF000)>>12))

// Arith decoder macros
#define ARITH_LEN_BYTES 4
#define ARITH_RM(addr)  (main_memory[addr+0] & 0x0FFF)
#define ARITH_MEM(addr) (main_memory[addr+1] & 0xFFFF)
#define ARITH_IMM(addr) (main_memory[addr+1] & 0xFFFF)


// Logic decoder macros
#define LOGIC_LEN_BYTES 6
#define LOGIC_RM(addr)  (main_memory[addr+0] & 0x0FFF)
#define LOGIC_MEM(addr) (main_memory[addr+1] & 0xFFFF)
#define LOGIC_IMM(addr) (main_memory[addr+2] & 0xFFFF)


void interp_instr() {
    mem_t ip = main_memory[INST_PTR_LOC];
    mem_t next_ip = -1;
    enum op opcode = OPCODE(ip);
    mem_t rm, imm, mem; // operands
    int16_t signed_val; // for SA
    switch (opcode) {
        case MI:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            imm = ARITH_IMM(ip);
            P_TRACE("MI 0x%04x, 0x%04x", rm, imm);
            main_memory[rm] = imm; 
            break;
        case MV:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("MV 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] = main_memory[mem];
            break;
        case MD:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("MD 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] = main_memory[main_memory[mem]];
            break;
        case LD:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("LD 0x%04x, 0x%04x", rm, mem);
            main_memory[main_memory[rm]] = main_memory[mem];
            break;
        case ST:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("ST 0x%04x, 0x%04x", rm, mem);
            main_memory[main_memory[mem]] = main_memory[rm];
            break;
        case AD:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("AD 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] += main_memory[mem];
            break;
        case SB:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("SB 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] -= main_memory[mem];
            break;
        case ND:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("ND 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] &= main_memory[mem];
            break;
        case OR:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("OR 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] |= main_memory[mem];
            break;
        case XR:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("XR 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] ^= main_memory[mem];
            break;
        case SR:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("SR 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] >>= main_memory[mem];
            break;
        case SL:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("SL 0x%04x, 0x%04x", rm, mem);
            main_memory[rm] <<= main_memory[mem];
            break;
        case SA:
            next_ip = ip + (ARITH_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = ARITH_RM(ip);
            mem = ARITH_MEM(ip);
            P_TRACE("SA 0x%04x, 0x%04x", rm, mem);
            signed_val = int16_t(main_memory[rm]);
            signed_val >>= main_memory[mem];
            main_memory[rm] = uint16_t(signed_val);
            break;
        case JG:
            next_ip = ip + (LOGIC_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = LOGIC_RM(ip);
            mem = LOGIC_MEM(ip);
            imm = LOGIC_IMM(ip);
            P_TRACE("JG 0x%04x, 0x%04x, 0x%04x", rm, mem, imm);
            if (main_memory[rm] > main_memory[mem]) {
                next_ip = imm;
                main_memory[INST_PTR_LOC] = next_ip;
            }
            break;
        case JL:
            next_ip = ip + (LOGIC_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = LOGIC_RM(ip);
            mem = LOGIC_MEM(ip);
            imm = LOGIC_IMM(ip);
            P_TRACE("JG 0x%04x, 0x%04x, 0x%04x", rm, mem, imm);
            if (main_memory[rm] < main_memory[mem]) {
                next_ip = imm;
                main_memory[INST_PTR_LOC] = next_ip;
            }
            break;
        case JQ:
            next_ip = ip + (LOGIC_LEN_BYTES>>1);
            main_memory[INST_PTR_LOC] = next_ip;
            rm = LOGIC_RM(ip);
            mem = LOGIC_MEM(ip);
            imm = LOGIC_IMM(ip);
            P_TRACE("JQ 0x%04x, 0x%04x, 0x%04x", rm, mem, imm);
            if (rm == 0 && mem == 0 && imm == ip) { // Actually an HF instruction
                P_TRACE("%s", "(actually HF)"); // the P_TRACE macro needs an VARARG, so do it this way :/
                printf("HF @ 0x%04x\n", ip);
                // TODO: switch to HACF CPU mode
                abort();
            } else if (main_memory[rm] == main_memory[mem]) {
                next_ip = imm;
                main_memory[INST_PTR_LOC] = next_ip;
            }
            break;
    }
    main_memory.flush_changes();
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Err: Specify file\n");
        return -1;
    }

    FILE* fd = fopen(argv[1], "r");
    if (!fd) {
        perror("File");
        return -1;
    }
    
#define MAX_CODE_SIZE 0xFFFF
    mem_t code[MAX_CODE_SIZE] = {0};
    size_t code_size = fread(code, 2, MAX_CODE_SIZE, fd);

    for (size_t i = 0; i < code_size; ++i) {
        main_memory[CODE_START + i] = code[i];
    }

    main_memory[INST_PTR_LOC] = CODE_START; // initialize instruction pointer
    main_memory[STK_PTR_LOC] = 0x300; // initialize stack pointer

    //setvbuf(stdin, NULL, _IONBF, 0); // unbuffer stdout
    //setvbuf(stdout, NULL, _IONBF, 0); // unbuffer stdout
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    while (1) {
        interp_instr();
    }
}
