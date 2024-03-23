#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include "emulator.h"
#include "assembler.h"
#include "utils.h"

#define HI_PURPLE "\e[0;95m"
#define HI_GREEN "\e[0;92m"
#define HI_YELLOW "\e[0;93m"
#define HI_RED "\e[0;91m"
#define COL_RESET "\x1b[0m"
#define SCREEN_CLEAR "\033[2J \033[H"

int startEmulator(char* ram) {
    CPU* cpu = initializeEmulator(ram);

    printf(HI_YELLOW
        HI_YELLOW "---------------------------------------------\n"
        HI_YELLOW "|   <LOL16 Architecture Emulator Console>   |\n"
        HI_YELLOW "---------------------------------------------\n"
        COL_RESET
    );

    bool isRunning = true;
    while (isRunning) {
        char command[100];
        printf(HI_PURPLE "LOL16> " COL_RESET);
        fflush(stdout);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        if (*command == '\0') { continue; }
        char* token = strtok(command, " ");
        
        if (strcmp(token, "exit") == 0) {
            printf(HI_RED "Exiting!\n" COL_RESET);
            isRunning = false;
        } else if (strcmp(token, "exec") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) { continue; }
            if (strlen(token) != 32) {
                printf(HI_RED "Number wrong size! Expected 32, got %zu\n" COL_RESET, strlen(token));
                continue;
            }
            uint32_t value = bin2dec(token);
            if (value == (unsigned int)-1) {
                printf(HI_RED "Cannot parse! Input is not a binary number!\n" COL_RESET);
                continue;
            }
            Instruction instruction = parseBytes(value);
            printf(SCREEN_CLEAR);
            printInstructionStruct(&instruction);
            executeInstruction(instruction, cpu, true);
        } else if (strcmp(token, "step") == 0) {
            printf(SCREEN_CLEAR);
            tickComputer(cpu, true);
        } else if (strcmp(token, "m") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) { continue; }
            char* endptr;
            unsigned long num = strtoul(token, &endptr, 10);
            if (strncmp(token, "0x", 2) == 0) {
                token[1] = '0';
                num = strtoul(token, &endptr, 16);
            }
            if (strncmp(token, "0b", 2) == 0) {
                token[1] = '0';
                num = strtoul(token, &endptr, 2);
            }
            if (*endptr != '\0' || endptr == token) {
                printf(HI_RED "Could not parse input address!\n" COL_RESET);
                continue;
            }
            if (num > UINT16_MAX) {
                printf(HI_RED "Input address does not fit within bounds of memory!\n" COL_RESET);
                continue;
            }

            uint16_t address = (uint16_t)num;
            for (int i = 0; i < 10; i++)
            {
                printf(HI_GREEN "%u : %u\n" COL_RESET, address+i, (uint32_t)cpu->ram[address+i] & 255);
            }
        }
    }

    free(cpu);
    return EXIT_SUCCESS;
}

int main(int argc, char const *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-r") == 0) {
            if (argc > 2) {
                char ram[65536];
                FILE* file = fopen(argv[2], "r");
                if (file == NULL) {
                    printf(HI_RED "Fatal error! Cannot find file %s\n" COL_RESET, argv[2]);
                    return EXIT_FAILURE;
                }
                char c;
                int i = 0;
                while ((c = fgetc(file)) != EOF) {
                    ram[i] = c;
                    i++;
                }
                fflush(stdout);

                return startEmulator(ram);
            } else {
                printf(HI_RED "Fatal error! No ram binary specified!\n" COL_RESET);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[1], "-a") == 0) {
            if (argc > 2) {
                FILE* file = fopen(argv[2], "rb");
                if (!file) {
                    printf(HI_RED "Source file not found!\n" COL_RESET);
                    return EXIT_FAILURE;
                }

                fseek(file, 0, SEEK_END);
                long size = ftell(file);
                rewind(file);

                char* src = (char*)malloc(size + 1); 
                if (!src) {
                    fclose(file);
                    return EXIT_FAILURE;
                }

                fread(src, 1, size, file);
                src[size] = '\0';
                fclose(file);

                char ram[65536];
                int status = assembleIntoRAM(src, ram, 0xA000);
                free(src);
                if (status == EXIT_FAILURE) { return EXIT_FAILURE; }
                return startEmulator(ram);
            } else {
                printf(HI_RED "Fatal error! No ram binary provided\n" COL_RESET);
                return EXIT_FAILURE;
            }
        } else {
            printf(HI_YELLOW "Usage: lol16 [-a -r] file\n" COL_RESET);
            return EXIT_FAILURE;
        }
    }
}
