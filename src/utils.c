#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "utils.h"
#include "assembler.h"

char lowByte(uint16_t num) {
    return (char)num&255;
}
char highByte(uint16_t num) {
    return (char)(num>>8)&255;
}

uint32_t hex2dec(const char *hex) {
    uint32_t result = 0;
    int i = 0;

    if (*hex == 0x00) {
        printf("Hello\n");
        return UINT32_MAX;
    }

    // Assuming hex is a null-terminated string
    while (hex[i] != '\0') {
        char c = hex[i];
        uint32_t digit_value;

        if (c >= '0' && c <= '9')
            digit_value = c - '0';
        else if (c >= 'a' && c <= 'f')
            digit_value = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            digit_value = c - 'A' + 10;
        else {
            return UINT32_MAX;
        }

        result = (result << 4) | digit_value;
        i++;
    }

    return result;
}

uint32_t bin2dec(const char *binary) {
    uint32_t result = 0;
    int length = strlen(binary);

    for (int i = 0; i < length; i++) {
        if (binary[i] == '1') {
            result = (result << 1) | 1; // Shift left by 1 and set the least significant bit to 1
        } else if (binary[i] == '0') {
            result = result << 1; // Shift left by 1 (no need to set the least significant bit)
        } else {
            return -1; // Error: Invalid binary digit
        }
    }

    return result;
}

// Function to convert a uint32_t value to big-endian format
uint32_t toBigEndian(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >>  8) |
           ((value & 0x0000FF00) <<  8) |
           ((value & 0x000000FF) << 24);
}
void switchPtrEndianess(uint16_t* ptr) {
    ptr[0] = ((ptr[0] & 0xFF00) >> 8) |
             ((ptr[0] & 0x00FF) << 8);
}

void printInstructionStruct(Instruction* instruction) {
    printf(
        HI_GREEN 
        "Instruction:\n"
        "   |- r1 - %d\n"
        "   |- r2 - %d\n"
        "   |- r3 - %d\n"
        "   |- upperBytes - %u\n"
        "   \\- opId - %u\n"
        COL_RESET, instruction->r1, instruction->r2, instruction->r3,
        instruction->data, instruction->opId
    );
}

char* matches(const char* input, const char* pattern) {
    pcre2_code *re;
    PCRE2_SPTR pattern_sptr = (PCRE2_SPTR)pattern;
    PCRE2_SIZE error_offset;
    int error_code;
    int len;
    char* matched = NULL;

    // Compile the regular expression
    re = pcre2_compile(pattern_sptr, PCRE2_ZERO_TERMINATED, 0, &error_code, &error_offset, NULL);
    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(error_code, buffer, sizeof(buffer));
        fprintf(stderr, "Error compiling regex at offset %d: %s\n", (int)error_offset, buffer);
        return NULL;
    }

    // Create matcher context
    pcre2_match_context *match_context = pcre2_match_context_create(NULL);

    // Create match data
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);

    // Execute the regular expression
    int rc = pcre2_match(re, (PCRE2_SPTR)input, strlen(input), 0, 0, match_data, match_context);
    if (rc < 0) {
        if (rc != PCRE2_ERROR_NOMATCH) {
            fprintf(stderr, "Error executing regex: %d\n", rc);
        }
        pcre2_match_data_free(match_data);
        pcre2_match_context_free(match_context);
        pcre2_code_free(re);
        return NULL; // No match or error
    }

    // Get the matched substring
    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    len = (int)(ovector[1] - ovector[0]);
    matched = (char*)malloc(len + 1);
    if (matched == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        pcre2_match_data_free(match_data);
        pcre2_match_context_free(match_context);
        pcre2_code_free(re);
        return NULL;
    }
    strncpy(matched, input + ovector[0], len);
    matched[len] = '\0';

    // Free memory and return
    pcre2_match_data_free(match_data);
    pcre2_match_context_free(match_context);
    pcre2_code_free(re);
    return matched;
}
