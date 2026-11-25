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

/* Traduz um endereco virtual (0 a 65535) e grava a linha em correct_fp:
 *   Virtual address: %d Physical address: %d Value: %d
 * g_clock e o relogio logico global (incrementado a cada acesso).
 * Retorna 1 em sucesso, 0 em erro (mensagem em stderr). */
int translate_address(MemoryManager *mm, int virtual_address,
                      ReplacementAlg page_alg, ReplacementAlg tlb_alg,
                      FILE *backing_store, FILE *correct_fp,
                      unsigned long *g_clock);

/* Escreve as 5 linhas finais de estatisticas no formato do correct.txt. */
void write_correct_summary(const MemoryManager *mm, FILE *correct_fp);

/* Gera o relatorio final (cabecalho, configuracao, totais e porcentagens)
 * em evidencias.log (modo append) e no terminal (stdout). */
void write_final_report(const MemoryManager *mm, const char *addr_file,
                        const char *page_alg_name, const char *tlb_alg_name);

#endif /* TRADU_REGIS_H */