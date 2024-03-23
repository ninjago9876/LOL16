#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "emulator.h"
#include "utils.h"

CPU* initializeEmulator(char* ram) {
    CPU* cpu = malloc(sizeof(CPU));
    memset(cpu, 0, sizeof(CPU));
    memcpy(cpu->ram, ram, sizeof(cpu->ram));
    cpu->PC = 0;
    return cpu;
}

Instruction parseBytes(uint32_t data) {
    Instruction instruction = {0};
    char regs = (data >> 24) & 0xFF;
    instruction.opId = (data >> 16) & 0xFF;
    instruction.data = (uint16_t)((((data >> 8) & 0xFF) << 8) | (data & 0xFF));

    instruction.r1 = (regs & 0b00000011);
    instruction.r2 = (regs & 0b00001100)>>2;
    instruction.r3 = (regs & 0b00110000)>>4;

    return instruction;
}

uint16_t getRegister(Register reg, CPU* cpu) {
    switch (reg) {
        case REG_A:
            return cpu->regA;
        case REG_X:
            return cpu->regX;
        case REG_Y:
            return cpu->regY;
        case REG_AX:
            return cpu->regAX;
    }
    return 0;
}
void setRegister(Register reg, uint16_t value, CPU* cpu) {
    switch (reg) {
        case REG_A:
            cpu->regA = value;
            break;
        case REG_X:
            cpu->regX = value;
            break;
        case REG_Y:
            cpu->regY = value;
            break;
        case REG_AX:
            cpu->regAX = value;
            break;
    }
}

void printCPUState(CPU* cpu) {
    printf(
        HI_GREEN 
        "\n"
        "   |- A - %u\n"
        "   |- X - %u\n"
        "   |- Y - %u\n"
        "   |- AX - %u\n"
        "   |- PC - %u\n"
        "   |- zero - %u\n"
        "   |- neg - %u\n"
        "   |- equ - %u\n"
        "   |- neq - %u\n"
        "   |- gr - %u\n"
        "   |- ge - %u\n"
        "   |- ls - %u\n"
        "   |- le - %u\n"
        "   \\- stackptr - %u\n"
        COL_RESET, cpu->regA, cpu->regX, cpu->regY, cpu->regAX, cpu->PC,
        cpu->zero, cpu->neq, cpu->equ, cpu->neq, cpu->gr, cpu->ge, cpu->ls, cpu->le, cpu->stackptr
    );
}

void tickComputer(CPU* cpu, bool verbose) {
    if (cpu->PC == 0) {
        cpu->PC = ((uint16_t)cpu->ram[0]<<8) | ((uint16_t)cpu->ram[1]);
        cpu->stackptr = cpu->PC - 1;
        printf(HI_YELLOW "\nCPU: " COL_RESET);
        printCPUState(cpu);
        printf("\n");
        return;
    }
    uint32_t bytes = ((uint32_t)cpu->ram[cpu->PC  ] & 0xFF)<<24 |
                     ((uint32_t)cpu->ram[cpu->PC+1] & 0xFF)<<16 |
                     ((uint32_t)cpu->ram[cpu->PC+2] & 0xFF)<<8  |
                     ((uint32_t)cpu->ram[cpu->PC+3] & 0xFF);
    Instruction instruction = parseBytes(bytes);
    if (verbose) { printInstructionStruct(&instruction); }
    cpu->PC += 4;
    executeInstruction(instruction, cpu, verbose);
}

void compare(uint16_t num1, uint16_t num2, CPU* cpu) {
    cpu->equ = (num1 == num2);
    cpu->neq = (num1 != num2);
    cpu->gr = (num1 > num2);
    cpu->ge = (num1 >= num2);
    cpu->ls = (num1 < num2);
    cpu->le = (num1 <= num2);
}

void executeInstruction(Instruction instruction, CPU* cpu, bool verbose) {
    Register r1 = instruction.r1;
    Register r2 = instruction.r2;
    Register r3 = instruction.r3;
    uint16_t value = instruction.data;
    uint16_t address = instruction.data;
    switch (instruction.opId) {
        case MOV_R_R: 
            setRegister(getRegister(r2, cpu), r1, cpu);
            break;
        case MOV_R_V:
            setRegister(r1, value, cpu);
            break;
        case MOV_R_A:
            setRegister(r1, ((uint16_t)cpu->ram[address])<<8 | (uint16_t)cpu->ram[address+1], cpu);
            break;
        case MOV_A_R:
            cpu->ram[address] = highByte(getRegister(r1, cpu));
            cpu->ram[address+1] = lowByte(getRegister(r1, cpu));
            break;
        case MOV_AR_R:
            cpu->ram[getRegister(r1, cpu)+1] = lowByte(getRegister(r2, cpu));
            cpu->ram[getRegister(r1, cpu)] = highByte(getRegister(r2, cpu));
            break;
        case MOV_AR_V:
            cpu->ram[getRegister(r1, cpu)+1] = lowByte(value);
            cpu->ram[getRegister(r1, cpu)] = highByte(value);
            break;
        case MOV_R_AR:
            setRegister(r1, cpu->ram[getRegister(r2, cpu)+1] + (cpu->ram[getRegister(r2, cpu)]<<8), cpu);
            break;
        case PUSH_R:
            cpu->ram[cpu->stackptr-1] = highByte(r1);
            cpu->ram[cpu->stackptr] = lowByte(r1);
            cpu->stackptr -= 2;
            break;
        case PUSH_V:
            cpu->ram[cpu->stackptr-1] = highByte(value);
            cpu->ram[cpu->stackptr] = lowByte(value);
            cpu->stackptr -= 2;
            break;
        case POP_R:
            cpu->stackptr += 2;
            setRegister(r1, cpu->ram[cpu->stackptr] + (cpu->ram[cpu->stackptr-1]<<8), cpu);
            break;
        case CALL_A:
            cpu->ram[cpu->stackptr-1] = highByte(cpu->PC+1);
            cpu->ram[cpu->stackptr] = lowByte(cpu->PC+1);
            cpu->stackptr -= 2;
            cpu->PC = address;
            break;
        case RET:
            cpu->PC = cpu->ram[cpu->stackptr] + (cpu->ram[cpu->stackptr-1]<<8);
            cpu->stackptr -= 2;
            break;
        case CMP_R_V:
            compare(getRegister(r1, cpu), value, cpu);
            break;
        case CMP_V_R:
            compare(value, getRegister(r1, cpu), cpu);
            break;
        case CMP_R_R:
            compare(getRegister(r1, cpu), getRegister(r2, cpu), cpu);
            break;
        
    }
    if (verbose) { printf(HI_YELLOW "\nCPU: " COL_RESET); }
    if (verbose) { printCPUState(cpu); }
    if (verbose) { printf("\n"); }
}