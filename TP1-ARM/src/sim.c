#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include "sim.h"

instruction_t instruction_table[] = {
    {OP_MASK, OP_HALT << 21, "HALT", handle_halt},
    {OP_MASK, OP_ADDS_EXT << 21, "ADDS_EXT", handle_adds_ext},
    {OP_MASK, OP_ADDS_IMM << 21, "ADDS_IMM", handle_adds_imm},
    {OP_MASK, OP_SUBS_EXT << 21, "SUBS_EXT", handle_subs_ext},
    {OP_MASK, OP_SUBS_IMM << 21, "SUBS_IMM", handle_subs_imm},
    {OP_MASK, OP_ANDS_SHIFT << 21, "ANDS_SHIFT", handle_ands_shift},
    {OP_MASK, OP_EOR_SHIFT << 21, "EOR_SHIFT", handle_eor_shift},
    {OP_MASK, OP_ORR_SHIFT << 21, "ORR_SHIFT", handle_orr_shift},
};

void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    printf("Executing instruction: 0x%08x at PC: 0x%016lx\n", instruction, CURRENT_STATE.PC);

    for (size_t i = 0; i < INSTRUCTION_TABLE_SIZE; i++) {
        instruction_t *entry = &instruction_table[i];
        if ((instruction & entry->mask) == entry->pattern) {
            entry->handler(instruction);
            return;
        }
    }

    // Instrucción no reconocida
    printf("Unknown instruction: 0x%08x\n", instruction);
    NEXT_STATE.PC += 4;
}

/*
    HANDLERS
*/

static void handle_halt(uint32_t instruction) {
    printf("[HALT]\n");
    RUN_BIT = 0;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

static void handle_adds_ext(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[ADDS] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
}

static void handle_adds_imm(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t imm = extract_field(instruction, IMM12_MASK, IMM12_SHIFT);
    uint8_t shift = extract_field(instruction, SHIFT1_MASK, SHIFT1_SHIFT);

    if(shift) imm <<= 12;
    
    uint64_t result = CURRENT_STATE.REGS[rn] + imm;
    NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[ADDS IMM] X%d, X%d, #0x%lx (shift: %d) => 0x%016lx\n", 
           rd, rn, imm, shift, result);
}

static void handle_subs_ext(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
    if(rd != 31) NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[SUBS/CMP] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
}

static void handle_subs_imm(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t imm = extract_field(instruction, IMM12_MASK, IMM12_SHIFT);
    uint8_t shift = extract_field(instruction, SHIFT1_MASK, SHIFT1_SHIFT);

    if(shift) imm <<= 12;
    
    uint64_t result = CURRENT_STATE.REGS[rn] - imm;
    if(rd != 31) NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[SUBS/CMP IMM] X%d, X%d, #0x%lx (shift: %d) => 0x%016lx\n",
           rd, rn, imm, shift, result);
}

static void handle_ands_shift(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);
    ShiftType shift_type = extract_field(instruction, SHIFT_TYPE_MASK, SHIFT_TYPE_SHIFT);
    uint32_t shift_amount = extract_field(instruction, IMM6_MASK, IMM6_SHIFT);

    uint64_t shifted = apply_shift(CURRENT_STATE.REGS[rm], shift_type, shift_amount);
    uint64_t result = CURRENT_STATE.REGS[rn] & shifted;
    
    NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[ANDS] X%d, X%d, X%d %s #%d => 0x%016lx\n",
           rd, rn, rm, shift_type_to_str(shift_type), shift_amount, result);
}

static void handle_eor_shift(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);
    ShiftType shift_type = extract_field(instruction, SHIFT_TYPE_MASK, SHIFT_TYPE_SHIFT);
    uint32_t shift_amount = extract_field(instruction, IMM6_MASK, IMM6_SHIFT);

    uint64_t shifted = apply_shift(CURRENT_STATE.REGS[rm], shift_type, shift_amount);
    uint64_t result = CURRENT_STATE.REGS[rn] ^ shifted;
    
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC += 4;
    printf("[EOR] X%d, X%d, X%d %s #%d => 0x%016lx\n",
           rd, rn, rm, shift_type_to_str(shift_type), shift_amount, result);
}

static void handle_orr_shift(uint32_t instruction) {
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);
    ShiftType shift_type = extract_field(instruction, SHIFT_TYPE_MASK, SHIFT_TYPE_SHIFT);
    uint32_t shift_amount = extract_field(instruction, IMM6_MASK, IMM6_SHIFT);

    uint64_t shifted = apply_shift(CURRENT_STATE.REGS[rm], shift_type, shift_amount);
    uint64_t result = CURRENT_STATE.REGS[rn] | shifted;
    
    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC += 4;
    printf("[ORR] X%d, X%d, X%d %s #%d => 0x%016lx\n",
           rd, rn, rm, shift_type_to_str(shift_type), shift_amount, result);
}

/*
    HELPER FUNCTIONS
*/

static inline uint32_t extract_field(uint32_t instruction, uint32_t mask, uint8_t shift) {
    return (instruction & mask) >> shift;
}

static uint64_t apply_shift(uint64_t value, ShiftType type, uint32_t amount) {
    if(amount == 0) return value;  // No shift needed
    
    switch(type) {
        case SHIFT_LSL: return value << amount;
        case SHIFT_LSR: return value >> amount;
        case SHIFT_ASR: return (int64_t)value >> amount;
        case SHIFT_ROR: return (value >> amount) | (value << (64 - amount));
        default:
            assert(0 && "Invalid shift type");
            return value;
    }
}

static void set_flags_z_n(CPU_State *state, uint64_t result) {
    state->FLAG_Z = (result == 0) ? 1 : 0;
    state->FLAG_N = (result >> 63) & 1;  // Usamos el bit más significativo
}

static const char* shift_type_to_str(ShiftType type) {
    static const char* names[] = {"LSL", "LSR", "ASR", "ROR"};
    return names[type];
}