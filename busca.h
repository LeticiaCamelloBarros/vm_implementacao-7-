#ifndef BUSCA_H
#define BUSCA_H

#include <pthread.h>

/* Numero de entradas da TLB (uma thread de busca por entrada). */
#define TLB_SIZE 16

/* Uma entrada da TLB: mapeia pagina virtual -> quadro fisico. */
typedef struct {
    int page_num;               /* numero da pagina virtual */
    int frame_num;              /* numero do quadro fisico */
    int valid;                  /* 1 = entrada em uso, 0 = vazia */
    unsigned long arrival_time; /* instante de insercao (FIFO) */
    unsigned long last_used;    /* instante do ultimo uso (LRU) */
} TLBEntry;

/* Argumentos entregues a cada thread de busca. */
typedef struct {
    int entry_index;            /* posicao da TLB (0 a 15) que esta thread verifica */
    int target_page;            /* pagina procurada */
    const TLBEntry *tlb_table;  /* tabela TLB (somente leitura) */
    int found;                  /* 1 se esta thread achou a pagina, 0 caso contrario */
    int frame_found;            /* quadro encontrado (valido se found == 1) */
} TLBSearchArgs;

/* Corpo de cada thread: verifica UMA entrada da TLB. */
void *search_tlb_entry(void *arg);

/* Dispara 16 threads, aguarda todas e retorna 1 (hit) ou 0 (miss).
 * Em caso de hit, preenche *found_frame e *found_index. */
int search_tlb_parallel(int page_num, const TLBEntry tlb[],
                        int *found_frame, int *found_index);

#endif 