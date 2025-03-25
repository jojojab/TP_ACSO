#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "shell.h"

// Definiciones mejoradas
#define OPCODE_MASK 0x1F000000
#define OPCODE_SHIFT 24

<<<<<<< Updated upstream
// Tabla de instrucciones
typedef struct
{
    uint32_t mask;
    uint32_t pattern;
    const char *name;
    void (*handler)(uint32_t);
} InstructionDesc;
=======
uint64_t rd;
uint64_t rn;
uint64_t imm;
uint32_t shift;
uint64_t rm;
uint64_t result_cmp;
>>>>>>> Stashed changes

// Prototipos
void handle_add(uint32_t instr);
void handle_sub(uint32_t instr);
void handle_movz(uint32_t instr);
void handle_halt(uint32_t instr);

InstructionDesc instructions[] = {
    {0xFFFFFC00, 0xD5030000, "HALT", handle_halt},
    {0xFFE00000, 0x4B000000, "ADDS (extended)", handle_add},
    {0xFFE00000, 0x6B000000, "SUBS (extended)", handle_sub},
    {0xFF800000, 0x52800000, "MOVZ", handle_movz},
};

int RUN_BIT = 1;
CPU_State CURRENT_STATE, NEXT_STATE;

void process_instruction()
{
    uint32_t instr = mem_read_32(CURRENT_STATE.PC);

    for (int i = 0; i < sizeof(instructions) / sizeof(InstructionDesc); i++)
    {
        if ((instr & instructions[i].mask) == instructions[i].pattern)
        {
            printf("Executing %s: 0x%08x\n", instructions[i].name, instr);
            instructions[i].handler(instr);

<<<<<<< Updated upstream
            if (RUN_BIT)
            {
                CURRENT_STATE = NEXT_STATE;
            }
            return;
        }
    }

    printf("Unknown instruction at PC=0x%08x: 0x%08x\n",
           CURRENT_STATE.PC, instr);
    RUN_BIT = 0;
=======

    printf("Executing instruction: 0x%08x\n", read);

    switch (instruction) {

        case 0x6a2:
            printf("HALT\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x558:
            printf("ADDS EXTENDED\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            rm = (read & 0x1F0000) >> 16;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

            NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0) ? 1 : 0;

            if (NEXT_STATE.REGS[rd] < 0) {
                NEXT_STATE.FLAG_N = 1;
            } else {
                NEXT_STATE.FLAG_N = 0;
            }

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x588:
            printf("ADDS IMMEDIATE\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            imm = (read & 0x3FFC00) >> 10;
            shift = (read & 0xC00000) >> 22;

//            printf("shift: 0x%02x\n", shift);

            if (shift == 1) {
                printf("Shifting...");
                imm = imm << 12;
            }

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm;

            NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0) ? 1 : 0;

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x758:

            printf("SUBS / CMP EXTENDED\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            rm = (read & 0x1F0000) >> 16;


            result_cmp = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

            if (rd != 31){
                NEXT_STATE.REGS[rd] = result_cmp;
            }

            NEXT_STATE.FLAG_Z = (result_cmp == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result_cmp < 0) ? 1 : 0;

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x788:

            printf("SUBS / CMP IMMEDIATE\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            imm = (read & 0x3FFC00) >> 10;
            shift = (read & 0xC00000) >> 22;

//            printf("shift: 0x%02x\n", shift);

            if (shift == 1) {
                printf("Shifting...");
                imm = imm << 12;
            }

            result_cmp = CURRENT_STATE.REGS[rn] - imm;

            if (rd != 31){
                NEXT_STATE.REGS[rd] = result_cmp;
            }

            NEXT_STATE.FLAG_Z = (result_cmp == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result_cmp < 0) ? 1 : 0;

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;


        case 0x750:

            printf("ANDS SHIFTED REGISTER\n");

            rm = (read & 0x1F0000) >> 16;
            rn = (read & 0x3E0) >> 5;
            rd = (read & 0x1F);
            imm = (read & 0xFC00) >> 10;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] & (CURRENT_STATE.REGS[rm]);

            NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0) ? 1 : 0;

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;


        case 0x650:

            printf("EOR SHIFTED REGISTER\n");

            rm = (read & 0x1F0000) >> 16;
            rn = (read & 0x3E0) >> 5;
            rd = (read & 0x1F);
            imm = (read & 0xFC00) >> 10;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] ^ (CURRENT_STATE.REGS[rm]);

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        default:
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

    }

    printf("Opcode: 0x%03x\n", instruction);


>>>>>>> Stashed changes
}

// Handlers específicos
void handle_add(uint32_t instr)
{
    uint32_t rd = (instr >> 0) & 0x1F;
    uint32_t rn = (instr >> 5) & 0x1F;
    uint32_t rm = (instr >> 16) & 0x1F;

    assert(rd < 32 && rn < 32 && rm < 32);

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

    NEXT_STATE = CURRENT_STATE;
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;
}

void handle_halt(uint32_t instr)
{
    RUN_BIT = 0;
}