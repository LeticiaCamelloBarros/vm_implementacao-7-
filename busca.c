#include "busca.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * Cada thread inspeciona apenas a sua entrada. Como cada uma escreve somente
 * na sua propria struct TLBSearchArgs e a TLB e lida (nunca escrita) durante
 * a busca, nao ha condicao de corrida e nao precisamos de mutex.
 */
void *search_tlb_entry(void *arg)
{
    TLBSearchArgs *a = (TLBSearchArgs *)arg;
    const TLBEntry *e = &a->tlb_table[a->entry_index];

    a->found = 0;
    a->frame_found = -1;

    if (e->valid == 1 && e->page_num == a->target_page) {
        a->found = 1;
        a->frame_found = e->frame_num;
    }

    pthread_exit(NULL);
}

int search_tlb_parallel(int page_num, const TLBEntry tlb[],
                        int *found_frame, int *found_index)
{
    pthread_t threads[TLB_SIZE];
    TLBSearchArgs args[TLB_SIZE];
    int created[TLB_SIZE];

    /* Fase 1: cria uma thread por entrada (fork). */
    for (int i = 0; i < TLB_SIZE; i++) {
        args[i].entry_index = i;
        args[i].target_page = page_num;
        args[i].tlb_table = tlb;
        args[i].found = 0;
        args[i].frame_found = -1;

        created[i] = (pthread_create(&threads[i], NULL,
                                     search_tlb_entry, &args[i]) == 0);
        if (!created[i]) {
            fprintf(stderr, "Erro: falha ao criar thread %d da TLB\n", i);
            /* Plano B: verifica essa entrada na thread atual para nao gerar
             * um falso miss. (Nao chamamos search_tlb_entry aqui porque o
             * pthread_exit encerraria a thread principal.) */
            const TLBEntry *e = &tlb[i];
            if (e->valid == 1 && e->page_num == page_num) {
                args[i].found = 1;
                args[i].frame_found = e->frame_num;
            }
        }
    }

    /* Fase 2: espera todas terminarem (join = barreira de sincronizacao). */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (created[i])
            pthread_join(threads[i], NULL);
    }

    /* Fase 3: le os resultados. */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (args[i].found == 1) {
            *found_frame = args[i].frame_found;
            *found_index = i;
            return 1; /* TLB Hit */
        }
    }
    return 0; /* TLB Miss */
}