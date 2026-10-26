// Copyright (C) 2026 Mitchell Scott
// SPDX-License-Identifier: GPL-3.0-only

#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "xovi.h"

#define LOG_PREFIX "[ebook-fit-to-height]"

// SceneView's default focal point for an ebook page, 3.28:
//   widthRatio = size.width / content.width
//   heightRatio = size.height / content.height
//   scale = min(widthRatio, heightRatio)
// 3.27 always scaled ebooks by the view height, so the fit branch is made unconditional
static const uint32_t ebookFitInstructions[] = {
    0x1e7d1bff, // fdiv  d31, d31, d29
    0x1e7c1842, // fdiv  d2, d2, d28
    0x1f5a7ba0, // fmadd d0, d29, d26, d30
    0x1f5a6f81, // fmadd d1, d28, d26, d27
    0x1e6223f0, // fcmpe d31, d2
    0x1e7fcc42, // fcsel d2, d2, d31, gt
};
#define EBOOK_FIT_INSTRUCTION_COUNT (sizeof(ebookFitInstructions) / sizeof(ebookFitInstructions[0]))
#define SELECT_SCALE_INDEX 5
#define NOP_INSTRUCTION 0xd503201f

struct CodeRange {
    uint8_t *start;
    size_t length;
};

static int isXochitlProcess(void)
{
    char executable[64] = {0};
    ssize_t length = readlink("/proc/self/exe", executable, sizeof(executable) - 1);
    return length > 0 && strcmp(executable, "/usr/bin/xochitl") == 0;
}

static int isRunningOnMove(void)
{
    char machine[64] = {0};
    FILE *machineFile = fopen("/sys/devices/soc0/machine", "r");
    if (!machineFile)
        return 0;
    size_t length = fread(machine, 1, sizeof(machine) - 1, machineFile);
    fclose(machineFile);
    machine[length] = '\0';
    return strstr(machine, "Chiappa") != NULL;
}

static int findMainProgramCode(struct dl_phdr_info *info, size_t size, void *data)
{
    (void)size;
    struct CodeRange *code = data;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *header = &info->dlpi_phdr[i];
        if (header->p_type == PT_LOAD && (header->p_flags & PF_X)) {
            code->start = (uint8_t *)(info->dlpi_addr + header->p_vaddr);
            code->length = header->p_memsz;
        }
    }
    return 1;
}

static uint32_t *findUniqueEbookFit(const struct CodeRange *code, int *matchCount)
{
    size_t patternBytes = sizeof(ebookFitInstructions);
    uint32_t *match = NULL;
    *matchCount = 0;
    for (size_t offset = 0; offset + patternBytes <= code->length; offset += 4) {
        if (memcmp(code->start + offset, ebookFitInstructions, patternBytes) == 0) {
            match = (uint32_t *)(code->start + offset);
            (*matchCount)++;
        }
    }
    return *matchCount == 1 ? match : NULL;
}

static int writeInstruction(uint32_t *location, uint32_t instruction)
{
    uintptr_t pageSize = (uintptr_t)sysconf(_SC_PAGESIZE);
    void *page = (void *)((uintptr_t)location & ~(pageSize - 1));
    if (mprotect(page, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
        return 0;
    *location = instruction;
    mprotect(page, pageSize, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char *)location, (char *)location + sizeof(instruction));
    return 1;
}

void _xovi_construct(void)
{
    if (!isXochitlProcess())
        return;

    if (!isRunningOnMove()) {
        fprintf(stderr, LOG_PREFIX " not a Paper Pro Move, staying inactive\n");
        return;
    }

    struct CodeRange code = {0};
    dl_iterate_phdr(findMainProgramCode, &code);
    if (!code.start) {
        fprintf(stderr, LOG_PREFIX " could not locate xochitl code, staying inactive\n");
        return;
    }

    int matchCount;
    uint32_t *ebookFit = findUniqueEbookFit(&code, &matchCount);
    if (!ebookFit) {
        fprintf(stderr, LOG_PREFIX " ebook fit code found %d times, unsupported xochitl build, staying inactive\n",
                matchCount);
        return;
    }

    if (!writeInstruction(ebookFit + SELECT_SCALE_INDEX, NOP_INSTRUCTION)) {
        fprintf(stderr, LOG_PREFIX " could not make xochitl code writable, staying inactive\n");
        return;
    }
    fprintf(stderr, LOG_PREFIX " ebooks now fit to view height\n");
}
