#include "TLB_ops.h"
#include "algo.h"
#include <stdio.h>
#include <stdlib.h>

/* Grava uma nova traducao em uma entrada da TLB. */
static void fill_entry(TLBEntry *e, int page_num, int frame_num,
                       unsigned long g_clock)
{
    e->page_num = page_num;
    e->frame_num = frame_num;
    e->valid = 1;
    e->arrival_time = g_clock; /* base do FIFO */
    e->last_used = g_clock;    /* base do LRU */
}

void update_tlb(TLBEntry tlb[], int page_num, int frame_num,
                ReplacementAlg tlb_alg, unsigned long g_clock)
{
    /* 1) Alocacao: se existe slot livre, usa-o (nao precisa despejar ninguem). */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb[i].valid) {
            fill_entry(&tlb[i], page_num, frame_num, g_clock);
            return;
        }
    }

    /* 2) TLB cheia: copia os tempos para vetores, que e o que as funcoes
     *    puras de algo.c recebem (assim servem tambem para a RAM). */
    unsigned long arrival_times[TLB_SIZE];
    unsigned long last_used_times[TLB_SIZE];
    for (int i = 0; i < TLB_SIZE; i++) {
        arrival_times[i] = tlb[i].arrival_time;
        last_used_times[i] = tlb[i].last_used;
    }

    /* 3) Escolhe a vitima conforme a politica (FIFO ou LRU). */
    int victim = select_victim_by_algorithm(tlb_alg, arrival_times,
                                            last_used_times, TLB_SIZE);
    if (victim < 0) {
        fprintf(stderr, "Erro: nao foi possivel escolher vitima na TLB\n");
        return;
    }

    /* 4) Substitui a vitima pela nova traducao. */
    fill_entry(&tlb[victim], page_num, frame_num, g_clock);
}

void invalidate_tlb_page(TLBEntry tlb[], int page_num)
{
    /* Se a pagina saiu da RAM, a TLB nao pode mais apontar para o quadro
     * antigo (ele sera reutilizado por outra pagina). */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_num == page_num)
            tlb[i].valid = 0;
    }
}