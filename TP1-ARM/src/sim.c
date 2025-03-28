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
    {OP_MASK, OP_STUR << 21, "STUR", handle_stur},
    {OP_MASK, OP_STURB << 21, "STURB", handle_sturb},
    {OP_MASK, OP_STURH << 21, "STURH", handle_sturh},
    {OP_MASK, OP_LDUR << 21, "LDUR", handle_ldur},
    {OP_MASK, OP_LDURB << 21, "LDURB", handle_ldurb},
    {OP_MASK, OP_LDURH << 21, "LDURH", handle_ldurh},
    {OP_MASK, OP_MOVZ << 23, "MOVZ", handle_movz},
    {OP_MASK_BRANCH, OP_BRANCH << 26, "BRANCH", handle_branch},
    {OP_MASK_BR, OP_BR << 10, "BR", handle_br},
    {OP_MASK_BCOND, OP_BCOND << 24, "BCOND", handle_bcond},
    {OP_MASK_LSL, OP_LSL << 23, "LSL", handle_lsl},
};

void process_instruction()
{
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

    for (size_t i = 0; i < INSTRUCTION_TABLE_SIZE; i++)
    {
        instruction_t *entry = &instruction_table[i];
        if ((instruction & entry->mask) == entry->pattern)
        {
            entry->handler(instruction);
            return;
        }
    }

    printf("Unknown instruction: 0x%08x\n", instruction);
    NEXT_STATE.PC += 4;
}

/*
    HANDLERS
*/

static void handle_halt(uint32_t instruction)
{
    printf("[HALT]\n");
    RUN_BIT = 0;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

static void handle_adds_ext(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[ADDS] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
}

static void handle_adds_imm(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t imm = extract_field(instruction, IMM12_MASK, IMM12_SHIFT);
    uint8_t shift = extract_field(instruction, SHIFT1_MASK, SHIFT1_SHIFT);

    if (shift)
        imm <<= 12;

    uint64_t result = CURRENT_STATE.REGS[rn] + imm;
    NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[ADDS IMM] X%d, X%d, #0x%lx (shift: %d) => 0x%016lx\n",
           rd, rn, imm, shift, result);
}

static void handle_subs_ext(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
    if (rd != 31)
        NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[SUBS/CMP] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
}

static void handle_subs_imm(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t imm = extract_field(instruction, IMM12_MASK, IMM12_SHIFT);
    uint8_t shift = extract_field(instruction, SHIFT1_MASK, SHIFT1_SHIFT);

    if (shift)
        imm <<= 12;

    uint64_t result = CURRENT_STATE.REGS[rn] - imm;
    if (rd != 31)
        NEXT_STATE.REGS[rd] = result;
    set_flags_z_n(&NEXT_STATE, result);

    NEXT_STATE.PC += 4;
    printf("[SUBS/CMP IMM] X%d, X%d, #0x%lx (shift: %d) => 0x%016lx\n",
           rd, rn, imm, shift, result);
}

static void handle_ands_shift(uint32_t instruction)
{
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

static void handle_eor_shift(uint32_t instruction)
{
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

static void handle_orr_shift(uint32_t instruction)
{
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

static void handle_stur(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    mem_write_32(address, CURRENT_STATE.REGS[rt]);

    NEXT_STATE.PC += 4;
    printf("[STUR] X%d, [X%d, #0x%lx] => 0x%016lx\n",
           rt, rn, offset, CURRENT_STATE.REGS[rt]);
}

static void handle_sturb(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint8_t data = CURRENT_STATE.REGS[rt] & 0xFF;
    mem_write_32(address, data);

    NEXT_STATE.PC += 4;
    printf("[STURB] X%d, [X%d, #0x%lx] => 0x%02x\n",
           rt, rn, offset, data);
}

static void handle_sturh(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint16_t data = CURRENT_STATE.REGS[rt] & 0xFFFF;
    mem_write_32(address, data);

    NEXT_STATE.PC += 4;
    printf("[STURH] X%d, [X%d, #0x%lx] => 0x%04x\n",
           rt, rn, offset, data);
}

static void handle_ldur(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint32_t data = mem_read_32(address);
    NEXT_STATE.REGS[rt] = data;

    NEXT_STATE.PC += 4;
    printf("[LDUR] X%d, [X%d, #0x%lx] => 0x%08x\n",
           rt, rn, offset, data);
}

static void handle_ldurb(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint8_t data = mem_read_32(address) & 0xFF;
    NEXT_STATE.REGS[rt] = (uint64_t)data;

    NEXT_STATE.PC += 4;
    printf("[LDURB] X%d, [X%d, #0x%lx] => 0x%02x\n",
           rt, rn, offset, data);
}

static void handle_ldurh(uint32_t instruction)
{
    uint8_t rt = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint64_t offset = extract_field(instruction, IMM9_MASK, IMM9_SHIFT);

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint16_t data = mem_read_32(address) & 0xFFFF;
    NEXT_STATE.REGS[rt] = (uint64_t)data;

    NEXT_STATE.PC += 4;
    printf("[LDURH] X%d, [X%d, #0x%lx] => 0x%04x\n",
           rt, rn, offset, data);
}

static void handle_movz(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint64_t imm = extract_field(instruction, 0x1FFFE0, 5);
    uint8_t shift = extract_field(instruction, 0x200000, 21);

    if (shift == 0)
    {
        NEXT_STATE.REGS[rd] = imm;
        set_flags_z_n(&NEXT_STATE, imm);
        printf("[MOVZ] X%d, #0x%lx => 0x%016lx\n", rd, imm, imm);
    }

    NEXT_STATE.PC += 4;
}

static void handle_branch(uint32_t instruction)
{
    uint32_t imm26 = extract_field(instruction, IMM26_MASK, 0);
    uint64_t offset = ((int64_t)(imm26 << 6)) >> 4;

    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    printf("[BRANCH] #%ld\n", offset);
}

static void handle_br(uint32_t instruction)
{
    uint8_t rm = extract_field(instruction, 0xFC0, 5);
    uint64_t offset = CURRENT_STATE.REGS[rm];
    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    printf("[BR] X%d => #%ld\n", rm, offset);
}

static void handle_bcond(uint32_t instruction)
{
    uint8_t cond = extract_field(instruction, COND_MASK, 0);
    uint32_t imm19 = extract_field(instruction, IMM19_MASK, 5);
    uint64_t offset = ((int32_t)(imm19 << 13)) >> 11;

    if (check_condition(cond))
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        printf("[BCOND] #%ld\n", offset);
    }
    else
    {
        NEXT_STATE.PC += 4;
        printf("[BCOND] Not taken\n");
    }
}

static void handle_lsl(uint32_t instruction)
{
    uint8_t rd = extract_field(instruction, RD_MASK, 0);
    uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
    uint8_t immr = extract_field(instruction, IMMR_MASK, IMMR_SHIFT);
    uint8_t imms = extract_field(instruction, IMMS_MASK, IMMS_SHIFT);

    uint64_t shifted = apply_shift(CURRENT_STATE.REGS[rn], SHIFT_LSL, 64 - immr);
    NEXT_STATE.REGS[rd] = shifted;
    NEXT_STATE.PC += 4;
    printf("[LSL] X%d, X%d, #%d, #%d\n", rd, rn, immr, imms);
}

/*
    HELPER FUNCTIONS
*/

static inline uint32_t extract_field(uint32_t instruction, uint32_t mask, uint8_t shift)
{
    return (instruction & mask) >> shift;
}

static uint64_t apply_shift(uint64_t value, ShiftType type, uint32_t amount)
{
    if (amount == 0)
        return value; // No shift needed

    switch (type)
    {
    case SHIFT_LSL:
        return value << amount;
    case SHIFT_LSR:
        return value >> amount;
    case SHIFT_ASR:
        return (int64_t)value >> amount;
    case SHIFT_ROR:
        return (value >> amount) | (value << (64 - amount));
    default:
        assert(0 && "Invalid shift type");
        return value;
    }
}

static void set_flags_z_n(CPU_State *state, uint64_t result)
{
    state->FLAG_Z = (result == 0) ? 1 : 0;
    state->FLAG_N = (result >> 63) & 1; // Usamos el bit más significativo
}

static const char *shift_type_to_str(ShiftType type)
{
    static const char *names[] = {"LSL", "LSR", "ASR", "ROR"};
    return names[type];
}

static bool check_condition(uint8_t cond)
{
    switch (cond)
    {
    case 0x0: // BEQ
        return CURRENT_STATE.FLAG_Z == 1;
    case 0x1: // BNE
        return CURRENT_STATE.FLAG_Z == 0;
    case 0x2: // BCS
        return FLAG_C == 1;
    case 0x3: // BCC
        return FLAG_C == 0;
    case 0x4: // BMI
        return CURRENT_STATE.FLAG_N == 1;
    case 0x5: // BPL
        return CURRENT_STATE.FLAG_N == 0;
    case 0x6: // BVS
        return FLAG_V == 1;
    case 0x7: // BVC
        return FLAG_V == 0;
    case 0x8: // BHI
        return (FLAG_C == 1 && CURRENT_STATE.FLAG_Z == 0);
    case 0x9: // BLS
        return !(FLAG_C == 1 && CURRENT_STATE.FLAG_Z == 0);
    case 0xA: // BGE
        return (CURRENT_STATE.FLAG_N == FLAG_V);
    case 0xB: // BLT
        return (CURRENT_STATE.FLAG_N != FLAG_V);
    case 0xC: // BGT
        return (CURRENT_STATE.FLAG_Z == 0 && (CURRENT_STATE.FLAG_N == FLAG_V));
    case 0xD: // BLE
        return (CURRENT_STATE.FLAG_Z == 0 && (CURRENT_STATE.FLAG_N != FLAG_V));
    case 0xE: // Undefined
        return false;
    case 0xF: // Always
        return true;

    default:
        assert(0 && "Invalid condition code");
        return false;
    }
}