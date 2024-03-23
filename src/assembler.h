#include "utils.h"

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

enum TokenType {
    REGISTER,
    IMMEDIATE,
    ADDRESS,
    INDIRECT_REGISTER
}; typedef enum TokenType TokenType;

struct Token {
    TokenType type;
    uint16_t value;
    Register reg;
}; typedef struct Token Token;

void printToken(Token* token);

int parseToken(const char *string, Token* token);

int parseInstruction(const char* string, Instruction* instruction, int line);

uint32_t toMachineCode(Instruction *instruction);

int assembleIntoRAM(char *src, char *ram, const uint16_t startVector);

#endif