#ifndef MF_PD_H
#define MF_PD_H

#include <stdio.h>
#include "TLB_ops.h"
#include "algo.h"

#define PAGE_SIZE     256
#define NUM_PAGES     256
#define NUM_FRAMES    128
#define PHYS_MEM_SIZE (NUM_FRAMES * PAGE_SIZE)  /* 32768 bytes = 32 KB */

/* Entrada da Tabela de Paginas (indexada pelo numero da pagina). */
typedef struct {
    int frame_num;              /* quadro da RAM onde a pagina esta */
    int valid;                  /* 0 = ausente da RAM, 1 = presente */
    unsigned long arrival_time; /* quando foi carregada (FIFO) */
    unsigned long last_used;    /* ultimo acesso (LRU) */
} PageTableEntry;

/* Escolhe qual dos 128 quadros sera descartado (FIFO ou LRU). */
int select_victim_frame(const unsigned long frame_arrival[],
                        const unsigned long frame_last_used[],
                        ReplacementAlg page_alg);

/* Le 256 bytes da pagina page_num do BACKING_STORE para o quadro frame_num.
 * Retorna 1 em sucesso, 0 em falha (erro impresso em stderr). */
int load_page_from_backing_store(FILE *backing_store, int page_num,
                                 int frame_num, signed char physical_memory[]);

/* Trata um Page Fault: aloca quadro livre ou despeja uma vitima, carrega a
 * pagina do disco e atualiza tabelas. Retorna o quadro usado, ou -1 em erro. */
int handle_page_fault(PageTableEntry page_table[], signed char physical_memory[],
                      int frame_to_page[], unsigned long frame_arrival[],
                      unsigned long frame_last_used[], int *allocated_frames,
                      int *page_faults, TLBEntry tlb[], int page_num,
                      ReplacementAlg page_alg, FILE *backing_store,
                      unsigned long g_clock);

#endif /* MF_PD_H */