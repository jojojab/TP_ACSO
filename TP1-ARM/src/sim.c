#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

#define OP_MASK 0xFFE00000

uint32_t rd;
uint32_t rn;
uint32_t imm;
uint32_t shift;
uint32_t rm;

int get_opcode(int instruction) {
    return (unsigned)(instruction & OP_MASK) >> 21;
}

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
            break;

        case 0x558:
            printf("ADDS EXTENDED\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            rm = (read & 0x1F0000) >> 16;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

            if (NEXT_STATE.REGS[rd] == 0) {
                NEXT_STATE.FLAG_Z = 1;
            } else {
                NEXT_STATE.FLAG_Z = 0;
            }

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
            imm = (read & 0x7C00) >> 10;
            shift = (read & 0xC00000) >> 22;

//            printf("shift: 0x%02x\n", shift);

            if (shift == 1) {
                printf("Shifting...");
                imm = imm << 12;
            }

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm;

            if (NEXT_STATE.REGS[rd] == 0) {
                NEXT_STATE.FLAG_Z = 1;
            } else {
                NEXT_STATE.FLAG_Z = 0;
            }

            if (NEXT_STATE.REGS[rd] < 0) {
                NEXT_STATE.FLAG_N = 1;
            } else {
                NEXT_STATE.FLAG_N = 0;
            }

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x758:

            printf("SUBS EXTENDED\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            rm = (read & 0x1F0000) >> 16;

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

            if (NEXT_STATE.REGS[rd] == 0) {
                NEXT_STATE.FLAG_Z = 1;
            } else {
                NEXT_STATE.FLAG_Z = 0;
            }

            if (NEXT_STATE.REGS[rd] < 0) {
                NEXT_STATE.FLAG_N = 1;
            } else {
                NEXT_STATE.FLAG_N = 0;
            }

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case 0x788:

            printf("SUBS IMMEDIATE\n");

            rd = (read & 0x1F);
            rn = (read & 0x3E0) >> 5;
            imm = (read & 0x7C00) >> 10;
            shift = (read & 0xC00000) >> 22;

//            printf("shift: 0x%02x\n", shift);

            if (shift == 1) {
                printf("Shifting...");
                imm = imm << 12;
            }

            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - imm;

            if (NEXT_STATE.REGS[rd] == 0) {
                NEXT_STATE.FLAG_Z = 1;
            } else {
                NEXT_STATE.FLAG_Z = 0;
            }

            if (NEXT_STATE.REGS[rd] < 0) {
                NEXT_STATE.FLAG_N = 1;
            } else {
                NEXT_STATE.FLAG_N = 0;
            }

            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;


        default:
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

    }

    printf("Opcode: 0x%03x\n", instruction);

//CURRENT_STATE.PC = NEXT_STATE;





}


