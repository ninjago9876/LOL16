#ifndef UTILS_H
#define UTILS_H

#define HI_PURPLE "\e[0;95m"
#define HI_GREEN "\e[0;92m"
#define HI_YELLOW "\e[0;93m"
#define HI_RED "\e[0;91m"
#define COL_RESET "\x1b[0m"
#define SCREEN_CLEAR "\033[2J \033[H"

enum Register {
    REG_A,
    REG_X,
    REG_Y,
    REG_AX
}; typedef enum Register Register;

struct Instruction {
    Register r1;
    Register r2;
    Register r3;
    uint16_t data;
    char opId;
}; typedef struct Instruction Instruction;

void printInstructionStruct(Instruction *instruction);

char lowByte(uint16_t num);

char highByte(uint16_t num);

uint32_t toBigEndian(uint32_t value);

void switchPtrEndianess(uint16_t *ptr);

char *matches(const char *input, const char *pattern);

unsigned int bin2dec(const char *binary);

uint32_t hex2dec(const char *hex);

enum Instructions {
    MOV_R_R,
    MOV_R_V,
    MOV_R_A,
    MOV_A_R,
    MOV_AR_R,
    MOV_AR_V,
    MOV_R_AR,
    PUSH_R,
    PUSH_V,
    POP_R,
    CALL_A,
    RET,
    CMP_R_V,
    CMP_V_R,
    CMP_R_R,
    JZ_A,
    JNZ_A,
    JN_A,
    JNN_A,
    JMP_A,
    JE_A,
    JNE_A,
    JL_A,
    JLE_A,
    JG_A,
    JGE_A,
    ADD_R_V_R,
    ADD_R_R_R,
    ADC_R_V_R,
    ADC_R_R_R,
    SUB_R_V_R,
    SUB_V_R_R,
    SUB_R_R_R,
    SBB_R_V_R,
    SBB_V_R_R,
    SBB_R_R_R,
    MUL_R_V_R,
    MUL_R_R_R,
    IMUL_R_V_R,
    IMUL_R_R_R,
    DIV_R_V_R,
    DIV_V_R_R,
    DIV_R_R_R,
    IDIV_R_V_R,
    IDIV_V_R_R,
    IDIV_R_R_R,
    PASS_R,
    HLT
}; typedef enum Instructions Instructions;

#endif