#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

#ifndef EMULATOR_H
#define EMULATOR_H

struct CPU {
    uint16_t regA;
    uint16_t regX;
    uint16_t regY;
    uint16_t regAX;
    uint16_t PC;
    bool zero;
    bool neg;
    bool equ;
    bool neq;
    bool gr;
    bool ge;
    bool ls;
    bool le;
    char ram[65536];
    uint16_t stackptr;
}; typedef struct CPU CPU;

CPU *initializeEmulator(char *ram);

void printCPUState(CPU *cpu);

Instruction parseBytes(uint32_t data);

void tickComputer(CPU *cpu, bool verbose);

void executeInstruction(Instruction instruction, CPU *cpu, bool verbose);

#endif
