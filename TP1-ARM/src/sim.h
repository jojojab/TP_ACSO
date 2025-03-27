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
#define SHIFT1_MASK    0x00400000
#define IMM6_MASK      0x0000FC00
#define SHIFT_TYPE_MASK 0x00C00000

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
#define B_TARGET        0x140

#define INSTRUCTION_TABLE_SIZE (sizeof(instruction_table)/sizeof(instruction_table[0]))

typedef struct {
    uint32_t mask;
    uint32_t pattern;
    const char* name;
    void (*handler)(uint32_t);
} instruction_t;

typedef enum {
    SHIFT_LSL = 0,
    SHIFT_LSR = 1,
    SHIFT_ASR = 2,
    SHIFT_ROR = 3
} ShiftType;

/* Declaración adelantada de funciones de manejo de instrucciones */
static void handle_halt(uint32_t instruction);
static void handle_adds_ext(uint32_t instruction);
static void handle_adds_imm(uint32_t instruction);
static void handle_subs_ext(uint32_t instruction);
static void handle_subs_imm(uint32_t instruction);
static void handle_ands_shift(uint32_t instruction);
static void handle_eor_shift(uint32_t instruction);
static void handle_orr_shift(uint32_t instruction);

static inline uint32_t extract_field(uint32_t instruction, uint32_t mask, uint8_t shift);
static uint64_t apply_shift(uint64_t value, ShiftType type, uint32_t amount);
static void set_flags_z_n(CPU_State *state, uint64_t result);
static const char* shift_type_to_str(ShiftType type);