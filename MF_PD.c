#include "MF_PD.h"
#include <stdio.h>
#include <stdlib.h>

int select_victim_frame(const unsigned long frame_arrival[],
                        const unsigned long frame_last_used[],
                        ReplacementAlg page_alg)
{
    /* Reaproveita a mesma logica generica usada pela TLB (modulo algo). */
    return select_victim_by_algorithm(page_alg, frame_arrival,
                                      frame_last_used, NUM_FRAMES);
}

int load_page_from_backing_store(FILE *backing_store, int page_num,
                                 int frame_num, signed char physical_memory[])
{
    /* Posiciona no inicio da pagina dentro do arquivo (pagina * 256). */
    if (fseek(backing_store, (long)page_num * PAGE_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: fseek falhou no BACKING_STORE (pagina %d)\n",
                page_num);
        return 0;
    }

    /* Le o bloco inteiro direto para o quadro, em uma unica operacao. */
    if (fread(&physical_memory[frame_num * PAGE_SIZE], sizeof(signed char),
              PAGE_SIZE, backing_store) != PAGE_SIZE) {
        fprintf(stderr, "Erro: fread falhou no BACKING_STORE (pagina %d)\n",
                page_num);
        return 0;
    }
    return 1;
}

int handle_page_fault(PageTableEntry page_table[], signed char physical_memory[],
                      int frame_to_page[], unsigned long frame_arrival[],
                      unsigned long frame_last_used[], int *allocated_frames,
                      int *page_faults, TLBEntry tlb[], int page_num,
                      ReplacementAlg page_alg, FILE *backing_store,
                      unsigned long g_clock)
{
    int frame_num;
    int used_free_frame = 0;

    (*page_faults)++;

    if (*allocated_frames < NUM_FRAMES) {
        /* Alocacao: ainda ha quadro nunca usado, entao nada e despejado. */
        frame_num = (*allocated_frames)++;
        used_free_frame = 1;
    } else {
        /* RAM cheia: escolhe um quadro vitima (FIFO ou LRU) e o despeja. */
        frame_num = select_victim_frame(frame_arrival, frame_last_used, page_alg);
        if (frame_num < 0) {
            fprintf(stderr, "Erro: nao foi possivel escolher quadro vitima\n");
            return -1;
        }

        int victim_page = frame_to_page[frame_num];
        if (victim_page >= 0) {
            /* A pagina despejada deixa de estar na RAM... */
            page_table[victim_page].valid = 0;
            /* ...e a TLB nao pode mais apontar para este quadro (coerencia). */
            invalidate_tlb_page(tlb, victim_page);
        }
        frame_to_page[frame_num] = -1;
    }

    /* Traz a pagina pedida do disco para o quadro escolhido. */
    if (!load_page_from_backing_store(backing_store, page_num, frame_num,
                                      physical_memory)) {
        if (used_free_frame)
            (*allocated_frames)--; /* devolve o quadro nao utilizado */
        return -1;
    }

    /* Atualiza a Tabela de Paginas. */
    page_table[page_num].frame_num = frame_num;
    page_table[page_num].valid = 1;
    page_table[page_num].arrival_time = g_clock;
    page_table[page_num].last_used = g_clock;

    /* Atualiza o mapeamento reverso quadro -> pagina e as marcas de tempo. */
    frame_to_page[frame_num] = page_num;
    frame_arrival[frame_num] = g_clock;
    frame_last_used[frame_num] = g_clock;

    return frame_num;
}