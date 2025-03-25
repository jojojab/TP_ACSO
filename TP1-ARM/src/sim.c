#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "shell.h"

// Definiciones mejoradas
#define OPCODE_MASK 0x1F000000
#define OPCODE_SHIFT 24

// Tabla de instrucciones
typedef struct
{
    uint32_t mask;
    uint32_t pattern;
    const char *name;
    void (*handler)(uint32_t);
} InstructionDesc;

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