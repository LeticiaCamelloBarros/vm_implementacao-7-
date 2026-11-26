#ifndef TRADU_REGIS_H
#define TRADU_REGIS_H

#include <stdio.h>
#include "MF_PD.h"
#include "TLB_ops.h"
#include "busca.h"
#include "algo.h"

/* Estado completo da simulacao. */
typedef struct {
    TLBEntry tlb[TLB_SIZE];
    PageTableEntry page_table[NUM_PAGES];
    signed char physical_memory[PHYS_MEM_SIZE];
    int frame_to_page[NUM_FRAMES];
    unsigned long frame_arrival[NUM_FRAMES];
    unsigned long frame_last_used[NUM_FRAMES];
    int allocated_frames;
    int total_addresses;
    int page_faults;
    int tlb_hits;
} MemoryManager;

/* Traduz um endereco virtual (0 a 65535) e grava em 'out' (stdout) a linha:
 *   Virtual address: %d TLB: %d Physical address: %d Value: %d
 * "TLB" e o indice (0..15) da entrada da TLB usada (hit) ou preenchida (miss).
 * g_clock e o relogio logico global (incrementado a cada acesso).
 * Retorna 1 em sucesso, 0 em erro (mensagem em stderr). */
int translate_address(MemoryManager *mm, int virtual_address,
                      ReplacementAlg page_alg, ReplacementAlg tlb_alg,
                      FILE *backing_store, FILE *out,
                      unsigned long *g_clock);

#endif /* TRADU_REGIS_H */
