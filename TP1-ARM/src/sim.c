#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

/* Máscaras y desplazamientos para campos de instrucción */
#define OP_MASK        0xFFE00000
#define RD_MASK        0x0000001F
#define RN_MASK        0x000003E0
#define RM_MASK        0x001F0000
#define IMM12_MASK     0x003FFC00
#define SHIFT1_MASK    0x00400000  // Campo de 1 bit para shift en instrucciones inmediatas
#define IMM6_MASK      0x0000FC00
#define SHIFT_TYPE_MASK 0x00C00000 // Campo de 2 bits para tipo de shift

/* Desplazamientos */
#define RN_SHIFT   5
#define RM_SHIFT   16
#define IMM12_SHIFT 10
#define SHIFT1_SHIFT 22
#define IMM6_SHIFT 10
#define SHIFT_TYPE_SHIFT 22

/* Opcodes */
#define OP_HALT         0x6A2
#define OP_ADDS_EXT     0x558
#define OP_ADDS_IMM     0x588
#define OP_SUBS_EXT     0x758
#define OP_SUBS_IMM     0x788
#define OP_ANDS_SHIFT   0x750
#define OP_EOR_SHIFT    0x650
#define OP_ORR_SHIFT    0x550

/* Tipos de desplazamiento */
typedef enum {
    SHIFT_LSL = 0,
    SHIFT_LSR = 1,
    SHIFT_ASR = 2,
    SHIFT_ROR = 3
} ShiftType;

/* Declaración adelantada de funciones helper */
static inline uint32_t extract_field(uint32_t instruction, uint32_t mask, uint8_t shift);
static uint64_t apply_shift(uint64_t value, ShiftType type, uint32_t amount);
static void set_flags_z_n(CPU_State *state, uint64_t result);
static const char* shift_type_to_str(ShiftType type);  // Declaración adelantada

void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction & OP_MASK) >> 21;

    printf("Executing instruction: 0x%08x at PC: 0x%016lx\n", 
           instruction, CURRENT_STATE.PC);

    switch(opcode) {
        case OP_HALT: {
            printf("[HALT]\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_ADDS_EXT: {
            uint8_t rd = extract_field(instruction, RD_MASK, 0);
            uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
            uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

            uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
            NEXT_STATE.REGS[rd] = result;
            set_flags_z_n(&NEXT_STATE, result);

            NEXT_STATE.PC += 4;
            printf("[ADDS] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
            break;
        }

        case OP_ADDS_IMM: {
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
            break;
        }

        case OP_SUBS_EXT: {
            uint8_t rd = extract_field(instruction, RD_MASK, 0);
            uint8_t rn = extract_field(instruction, RN_MASK, RN_SHIFT);
            uint8_t rm = extract_field(instruction, RM_MASK, RM_SHIFT);

            uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
            if(rd != 31) NEXT_STATE.REGS[rd] = result;
            set_flags_z_n(&NEXT_STATE, result);

            NEXT_STATE.PC += 4;
            printf("[SUBS/CMP] X%d, X%d, X%d => 0x%016lx\n", rd, rn, rm, result);
            break;
        }

        case OP_SUBS_IMM: {
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
            break;
        }

        case OP_ANDS_SHIFT: {
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
            break;
        }

        case OP_EOR_SHIFT: {
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
            break;
        }

        case OP_ORR_SHIFT: {
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
            break;
        }

        default:
            printf("Unknown instruction: 0x%03x\n", opcode);
            NEXT_STATE.PC += 4;
            break;
    }
}

/* Funciones helper implementación */
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

/* Función helper para depuración */
static const char* shift_type_to_str(ShiftType type) {
    static const char* names[] = {"LSL", "LSR", "ASR", "ROR"};
    return names[type];
}