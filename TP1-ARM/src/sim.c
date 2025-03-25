#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "shell.h"

// Constantes para manipulación de bits
#define OPCODE_MASK 0x1F000000
#define OPCODE_SHIFT 24
#define REGISTER_MASK 0x1F
#define IMMEDIATE_12_MASK 0xFFF
#define OFFSET_26_MASK 0x3FFFFFF
#define SHIFT_AMOUNT_MASK 0x3
#define SHIFT_TYPE_MASK 0x3

// Constantes para registros especiales
#define ZERO_REGISTER 31
#define LINK_REGISTER 30

typedef enum
{
    CAT_R,      // Registro-Registro
    CAT_I,      // Inmediato
    CAT_D,      // Load/Store
    CAT_B,      // Salto incondicional
    CAT_CB,     // Salto condicional
    CAT_IW,     // Inmediato ancho
    CAT_UNKNOWN // Instrucción desconocida
} InstructionCategory;

typedef struct
{
    InstructionCategory category;
    uint32_t mask;
    uint32_t pattern;
    const char *name;
    void (*handler)(uint32_t);
} InstructionInfo;

// Prototipos de funciones
void handle_r_type(uint32_t instr);
void handle_i_type(uint32_t instr);
void handle_d_type(uint32_t instr);
void handle_b_type(uint32_t instr);
void handle_cb_type(uint32_t instr);
void handle_iw_type(uint32_t instr);
void handle_adds_extended(uint32_t instr);
void handle_subs_extended(uint32_t instr);
void handle_ands_shifted(uint32_t instr);
void handle_eor_shifted(uint32_t instr);
void handle_adds_immediate(uint32_t instr);
void handle_subs_immediate(uint32_t instr);
void handle_halt(uint32_t instr);

// Tabla de instrucciones
static const InstructionInfo instruction_set[] = {
    // Instrucciones de Datos - Registro (Categoría R)
    {CAT_R, 0xFFE0FC00, 0x4B000000, "ADDS (extended)", handle_adds_extended},
    {CAT_R, 0xFFE0FC00, 0x6B000000, "SUBS/CMP (extended)", handle_subs_extended},
    {CAT_R, 0xFFE0FC00, 0x4A000000, "ANDS (shifted register)", handle_ands_shifted},
    {CAT_R, 0xFFE0FC00, 0x4A200000, "EOR (shifted register)", handle_eor_shifted},

    // Instrucciones de Datos - Inmediato (Categoría I)
    {CAT_I, 0xFF800000, 0x31000000, "ADDS (immediate)", handle_adds_immediate},
    {CAT_I, 0xFF800000, 0x71000000, "SUBS/CMP (immediate)", handle_subs_immediate},

    // Instrucciones Especiales
    {CAT_UNKNOWN, 0xFFFFFFFF, 0xD4000000, "HALT", handle_halt}};

static const size_t instruction_set_size = sizeof(instruction_set) / sizeof(InstructionInfo);

int RUN_BIT = 1;
CPU_State CURRENT_STATE, NEXT_STATE;

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
    uint32_t instr = mem_read_32(CURRENT_STATE.PC);
    if (instr == 0)
    {
        printf("Null instruction at PC=0x%016lx\n", CURRENT_STATE.PC);
        RUN_BIT = 0;
        return;
    }

    InstructionInfo *info = decode_instruction(instr);
    if (!info)
    {
        printf("Unknown instruction at PC=0x%016lx: 0x%08x\n",
               CURRENT_STATE.PC, instr);
        RUN_BIT = 0;
        return;
    }

    printf("Executing %s (Category %d): 0x%08x\n",
           info->name, info->category, instr);

    // Copiar estado actual antes de modificarlo
    NEXT_STATE = CURRENT_STATE;

    // Ejecutar handler específico
    info->handler(instr);

    // Actualizar estado si todo fue bien
    if (RUN_BIT)
    {
        CURRENT_STATE = NEXT_STATE;
    }
    else
    {
        printf("Simulation halted\n");
    }
}

/**
 * Valida que los registros sean válidos
 * @param ... Lista de registros a validar
 */
#define VALIDATE_REGISTERS(...)                                        \
    do                                                                 \
    {                                                                  \
        uint32_t regs[] = {__VA_ARGS__};                               \
        for (size_t i = 0; i < sizeof(regs) / sizeof(uint32_t); i++)   \
        {                                                              \
            if (regs[i] >= 32 && regs[i] != ZERO_REGISTER)             \
            {                                                          \
                printf("Error: Invalid register X%d at PC=0x%016lx\n", \
                       regs[i], CURRENT_STATE.PC);                     \
                RUN_BIT = 0;                                           \
                return;                                                \
            }                                                          \
        }                                                              \
    } while (0)

// Implementaciones de los handlers
void handle_adds_extended(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t rm = (instr >> 16) & REGISTER_MASK;

    VALIDATE_REGISTERS(rd, rn, rm);

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;

    printf("ADDS EXTENDED: X%d = X%d + X%d = 0x%016lx\n",
           rd, rn, rm, result);
}

void handle_subs_extended(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t rm = (instr >> 16) & REGISTER_MASK;

    VALIDATE_REGISTERS(rd, rn, rm);

    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

    // CMP es SUBS cuando rd = ZERO_REGISTER
    if (rd != ZERO_REGISTER)
    {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;

    printf("SUBS/CMP EXTENDED: X%d = X%d - X%d = 0x%016lx\n",
           rd, rn, rm, result);
}

void handle_ands_shifted(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t rm = (instr >> 16) & REGISTER_MASK;
    uint32_t shift = (instr >> 22) & SHIFT_AMOUNT_MASK;

    VALIDATE_REGISTERS(rd, rn, rm);

    uint64_t op2 = CURRENT_STATE.REGS[rm];
    if (shift != 0)
    {
        op2 <<= (shift * 16); // LSL #0, #16, #32, #48
    }

    uint64_t result = CURRENT_STATE.REGS[rn] & op2;

    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;

    printf("ANDS SHIFTED REGISTER: X%d = X%d & X%d = 0x%016lx\n",
           rd, rn, rm, result);
}

void handle_eor_shifted(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t rm = (instr >> 16) & REGISTER_MASK;
    uint32_t shift = (instr >> 22) & SHIFT_AMOUNT_MASK;

    VALIDATE_REGISTERS(rd, rn, rm);

    uint64_t op2 = CURRENT_STATE.REGS[rm];
    if (shift != 0)
    {
        op2 <<= (shift * 16);
    }

    uint64_t result = CURRENT_STATE.REGS[rn] ^ op2;

    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.PC += 4;

    printf("EOR SHIFTED REGISTER: X%d = X%d ^ X%d = 0x%016lx\n",
           rd, rn, rm, result);
}

void handle_adds_immediate(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t imm = (instr >> 10) & IMMEDIATE_12_MASK;
    uint32_t shift = (instr >> 22) & SHIFT_TYPE_MASK;

    VALIDATE_REGISTERS(rd, rn);

    if (shift == 1)
    {
        imm <<= 12; // LSL #12
    }

    uint64_t result = CURRENT_STATE.REGS[rn] + imm;

    NEXT_STATE.REGS[rd] = result;
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;

    printf("ADDS IMMEDIATE: X%d = X%d + 0x%x = 0x%016lx\n",
           rd, rn, imm, result);
}

void handle_subs_immediate(uint32_t instr)
{
    uint32_t rd = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t imm = (instr >> 10) & IMMEDIATE_12_MASK;
    uint32_t shift = (instr >> 22) & SHIFT_TYPE_MASK;

    VALIDATE_REGISTERS(rd, rn);

    if (shift == 1)
    {
        imm <<= 12; // LSL #12
    }

    uint64_t result = CURRENT_STATE.REGS[rn] - imm;

    if (rd != ZERO_REGISTER)
    {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result >> 63) & 1;
    NEXT_STATE.PC += 4;

    printf("SUBS/CMP IMMEDIATE: X%d = X%d - 0x%x = 0x%016lx\n",
           rd, rn, imm, result);
}

void handle_d_type(uint32_t instr)
{
    uint32_t rt = instr & REGISTER_MASK;
    uint32_t rn = (instr >> 5) & REGISTER_MASK;
    uint32_t offset = (instr >> 10) & IMMEDIATE_12_MASK;
    uint32_t opcode = (instr >> 22) & SHIFT_TYPE_MASK;

    VALIDATE_REGISTERS(rt, rn);

    uint64_t address = CURRENT_STATE.REGS[rn] + (offset << 3); // *8 para 64-bit

    switch (opcode)
    {
    case 0: // STR
        if (mem_write_64(address, CURRENT_STATE.REGS[rt]))
        {
            printf("Memory write failed at address 0x%016lx\n", address);
            RUN_BIT = 0;
            return;
        }
        break;
    case 1: // LDR
        NEXT_STATE.REGS[rt] = mem_read_64(address);
        if (NEXT_STATE.REGS[rt] == (uint64_t)-1)
        {
            printf("Memory read failed at address 0x%016lx\n", address);
            RUN_BIT = 0;
            return;
        }
        break;
    default:
        printf("Invalid D-type opcode: %d\n", opcode);
        RUN_BIT = 0;
        return;
    }

    NEXT_STATE.PC += 4;
}

void handle_b_type(uint32_t instr)
{
    int32_t offset = (instr & OFFSET_26_MASK) << 2; // Sign-extend and *4
    uint32_t opcode = (instr >> 31) & 0x1;

    if (opcode == 0)
    { // B
        NEXT_STATE.PC += offset;
    }
    else
    { // BL
        NEXT_STATE.REGS[LINK_REGISTER] = CURRENT_STATE.PC + 4;
        NEXT_STATE.PC += offset;
    }
}

void handle_halt(uint32_t instr)
{
    printf("HALT\n");
    RUN_BIT = 0;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}