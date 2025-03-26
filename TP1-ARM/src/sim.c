#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

#define OP_MASK 0xFFE00000

uint64_t rd;
uint64_t rn;
uint64_t imm;
uint32_t shift;
uint64_t rm;
uint64_t result_cmp;

int get_opcode(int instruction) {
    return (unsigned)(instruction & OP_MASK) >> 21;
}

/**
 * Decodifica una instrucción y devuelve su información
 * @param instr Instrucción a decodificar
 * @return Puntero a la información de la instrucción o NULL si no se reconoce
 */
InstructionInfo *decode_instruction(uint32_t instr)
{
    for (size_t i = 0; i < instruction_set_size; i++)
    {
        if ((instr & instruction_set[i].mask) == instruction_set[i].pattern)
        {
            return (InstructionInfo *)&instruction_set[i];
        }
    }
    return NULL;
}

/**
 * Procesa la instrucción actual
 */
void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory.
     * */

    uint32_t read = mem_read_32(CURRENT_STATE.PC);
    int instruction = get_opcode(read);


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


        case 0x550:

            printf("ORR SHIFTED REGISTER\n");

            rm = (read & 0x1F0000) >> 16;
            rn = (read & 0x3E0) >> 5;
            rd = (read & 0x1F);
            imm = (read & 0xFC00) >> 10;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] | (CURRENT_STATE.REGS[rm]);

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        default:
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

    }

    printf("Executing %s (Category %d): 0x%08x\n",
           info->name, info->category, instr);

    // Copiar estado actual antes de modificarlo
    NEXT_STATE = CURRENT_STATE;

}


