#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "assembler.h"

void printToken(Token* token) {
    printf(
        HI_GREEN 
        "Token:\n"
        "   |- type - %d\n"
        "   |- value - %u\n"
        "   \\- reg - %d\n"
        COL_RESET, token->type, token->value, token->reg
    );
}

void printMachineCode(uint32_t machineCode) {
    // Printing machine code in binary format
    for(int i = 31; i >= 0; i--) {
        printf("%d", (machineCode >> i) & 1);
        if (i%8==0 && i!=0) {
            printf(" ");
        }
    }
}

int parseToken(const char* string, Token* token) {
    if (string == NULL) {
        return 1;
    }
    char* current = (char*)string;
    token->type = 2;

    bool hasBrackets = matches(string, "^\\[.*\\]$");

    if (matches(current, "^\\[?(AX|Ax|aX|ax)\\]?$")) {
        token->type = hasBrackets ? INDIRECT_REGISTER : REGISTER;
        token->reg = REG_AX;
    } else if (matches(current, "^\\[?(A|a)\\]?$")) {
        token->type = hasBrackets ? INDIRECT_REGISTER : REGISTER;
        token->reg = REG_A;
    } else if (matches(current, "^\\[?(X|x)\\]?$")) {
        token->type = hasBrackets ? INDIRECT_REGISTER : REGISTER;
        token->reg = REG_X;
    } else if (matches(current, "^\\[?(Y|y)\\]?$")) {
        token->type = hasBrackets ? INDIRECT_REGISTER : REGISTER;
        token->reg = REG_Y;
    }

    if (token->type != 2) {
        return 0;
    }
    token->reg = 0;
    if (matches(current, "^#")) {
        token->type = IMMEDIATE;
        current++;
    } else {
        token->type = ADDRESS;
    }

    if (matches(current, "^[[:digit:]]+$")) {
        token->value = (uint16_t)strtoul(current, NULL, 10);
        return 0;
    } else if (matches(current, "^\\$[[:xdigit:]]+$")) {
        token->value = (uint16_t)strtoul(current+1, NULL, 16);
        return 0;
    } else if (matches(current, "^0b[01]+$")) {
        token->value = (uint16_t)strtoul(current+2, NULL, 2);
        return 0;
    }
    printf(HI_RED "Error parsing token! %s is not a valid value!\n" COL_RESET, current);
    return 2;
}

int checkTokenCount(int desiredTokenCount, int tokenCount) {
    if (tokenCount > desiredTokenCount) {
        printf(HI_RED "Too many operands for instruction!\n" COL_RESET);
        return 1;
    }
    if (tokenCount < desiredTokenCount) {
        printf(HI_RED "Too few valid operands for instruction!\n" COL_RESET);
        return 1;
    }
    return 0;
}
int parseLine(const char* string, Instruction* instruction, int line) {
    char* tokens[4];
    char current[100];
    strncpy(current, string, strlen(string)+1);
    if (matches(current, "^[[:space:]]*$")) {
        return 3;
    }
    int i = 0;
    tokens[i] = strtok(current, " \t,");
    while (tokens[i] != NULL) {
        i++;
        tokens[i] = strtok(NULL, ", \t");
    }

    char* opcode = tokens[0];
    if (opcode == NULL) { return 1; }
    int tokenCount = i;
    Token t1;
    if (tokenCount > 1) { parseToken(tokens[1], &t1); }
    Token t2;
    if (tokenCount > 2) { parseToken(tokens[2], &t2); }
    Token t3;
    if (tokenCount > 3) { parseToken(tokens[3], &t3); }

    if (strcmp(opcode, "mov") == 0) {
        if (checkTokenCount(3, tokenCount)) { return 2; }
        if (t1.type == REGISTER && t2.type == REGISTER) {
            instruction->opId = MOV_R_R;
            instruction->r1 = t1.reg;
            instruction->r2 = t2.reg;
            return 0;
        } else if (t1.type == REGISTER && t2.type == IMMEDIATE) {
            instruction->opId = MOV_R_V;
            instruction->r1 = t1.reg;
            instruction->data = t2.value;
            return 0;
        } else if (t1.type == REGISTER && t2.type == ADDRESS) {
            instruction->opId = MOV_R_A;
            instruction->r1 = t1.reg;
            instruction->data = t2.value;
            return 0;
        } else if (t1.type == ADDRESS && t2.type == REGISTER) {
            instruction->opId = MOV_A_R;
            instruction->r1 = t2.reg;
            instruction->data = t1.value;
            return 0;
        }else if (t1.type == INDIRECT_REGISTER && t2.type == REGISTER) {
            instruction->opId = MOV_AR_R;
            instruction->r1 = t1.reg;
            instruction->r2 = t2.reg;
            return 0;
        } else if (t1.type == INDIRECT_REGISTER && t2.type == IMMEDIATE) {
            instruction->opId = MOV_AR_V;
            instruction->r1 = t1.reg;
            instruction->data = t2.value;
            return 0;
        } else if (t1.type == REGISTER && t2.type == INDIRECT_REGISTER) {
            instruction->opId = MOV_R_AR;
            instruction->r1 = t1.reg;
            instruction->r2 = t2.reg;
            return 0;
        }
    } else if (strcmp(opcode, "push") == 0) {
        if (checkTokenCount(2, tokenCount)) { return 2; }
        if (t1.type == REGISTER) {
            instruction->opId = PUSH_R;
            instruction->r1 = t1.reg;
            return 0;
        } else if (t1.type == IMMEDIATE) {
            instruction->opId = PUSH_V;
            instruction->data = t1.value;
            return 0;
        }
    } else if (strcmp(opcode, "pop") == 0) {
        if (checkTokenCount(2, tokenCount)) { return 2; }
        if (t1.type == REGISTER) {
            instruction->opId = POP_R;
            instruction->r1 = t1.reg;
            return 0;
        }
    } else if (strcmp(opcode, "call") == 0) {
        if (checkTokenCount(2, tokenCount)) { return 2; }
        if (t1.type == ADDRESS) {
            instruction->opId = CALL_A;
            instruction->data = t1.value;
            return 0;
        }
    } else if (strcmp(opcode, "ret") == 0) {
        if (checkTokenCount(1, tokenCount)) { return 2; }
        instruction->opId = RET;
        instruction->data = 0;
        instruction->r1 = 0;
        return 0;
    } else if (strcmp(opcode, "cmp") == 0) {
        if (checkTokenCount(3, tokenCount)) { return 2; }
        if (t1.type == REGISTER && t2.type == IMMEDIATE) {
            instruction->opId = CMP_R_V;
            instruction->r1 = t1.reg;
            instruction->data = t2.value;
            return 0;
        } else if (t1.type == IMMEDIATE && t2.type == REGISTER) {
            instruction->opId = CMP_V_R;
            instruction->r1 = t2.reg;
            instruction->data = t1.value;
            return 0;
        } else if (t1.type == REGISTER && t2.type == REGISTER) {
            instruction->opId = CMP_R_R;
            instruction->r1 = t1.reg;
            instruction->r2 = t2.reg;
            return 0;
        }
    }
    printf(HI_RED "%d: %s is not recognized!\n" COL_RESET, line+1, string);
    return 1;
}

uint32_t toMachineCode(Instruction* instruction) {
    uint8_t byte1 = (instruction->r1&0b11) | (instruction->r2&0b11)<<2 | (instruction->r3&0b11)<<4;
    uint8_t byte2 = instruction->opId;
    uint8_t byte3 = (char)((instruction->data>>8)&0xff);
    uint8_t byte4 = (char)(instruction->data&0xff);
    return byte4 | byte3<<8 | byte2<<16 | byte1<<24;
}

int assembleIntoRAM(char* src, char* ram, const uint16_t startVector) {
    uint32_t instructions[65536];
    FILE* file = fmemopen(src, strlen(src), "r");
    char string[100];
    int line = 0;
    while (fgets(string, sizeof(string), file) != NULL) {
        if (string[strlen(string)-1] == '\n') {
            string[strlen(string)-1] = '\0';
        }
        Instruction instruction;
        int status;
        if ((status = parseLine(string, &instruction, line)) != 0) {
            if (status == 3) {
                continue;
            } return EXIT_FAILURE;
        }
        printf("Assembling %d: %s   output: ", line+1, string);
        instructions[line] = toBigEndian(toMachineCode(&instruction));
        printMachineCode(toBigEndian(instructions[line]));
        printf("\n");
        line++;
    }

    uint16_t org = startVector;
    switchPtrEndianess(&org);
    memcpy(ram, &org, 2);
    switchPtrEndianess(&org);
    char buffer[(int)org-2];
    memset(buffer, 0, (int)org-2);
    memcpy(ram+2, buffer, sizeof(buffer));
    int i = 0;
    while (i < line) {
        ram[i*4+org] = (char)(instructions[i] & 0x000000ff);
        ram[i*4+org+1] = (char)((instructions[i] & 0x0000ff00) >> 8);
        ram[i*4+org+2] = (char)((instructions[i] & 0x00ff0000) >> 16);
        ram[i*4+org+3] = (char)((instructions[i] & 0xff000000) >> 24);
        i++; 
    }
    uint16_t size = org+i*4;
    char buffer1[65536-(int)size];
    memset(buffer1, 0, sizeof(buffer1));
    memcpy(ram+size, buffer1, sizeof(buffer1));
    return EXIT_SUCCESS;
}