#include "algo.h"

/* Funcoes puras: nao alteram os vetores nem usam variaveis globais,
 * por isso o mesmo codigo serve para a TLB e para a RAM fisica. */

/* Procura o indice do menor valor do vetor (busca linear, O(n)). */
static int index_of_min(const unsigned long times[], int size)
{
    int victim = 0;

    for (int i = 1; i < size; i++) {
        if (times[i] < times[victim])
            victim = i;
    }
    return victim;
}

int select_victim_fifo(const unsigned long arrival_times[], int size)
{
    if (size <= 0)
        return -1;
    return index_of_min(arrival_times, size); /* quem chegou primeiro */
}

int select_victim_lru(const unsigned long last_used_times[], int size)
{
    if (size <= 0)
        return -1;
    return index_of_min(last_used_times, size); /* quem ficou mais tempo sem uso */
}

int select_victim_by_algorithm(ReplacementAlg alg,
                               const unsigned long arrival_times[],
                               const unsigned long last_used_times[],
                               int size)
{
    switch (alg) {
    case ALG_FIFO:
        return select_victim_fifo(arrival_times, size);
    case ALG_LRU:
        return select_victim_lru(last_used_times, size);
    default:
        return -1;
    }
}
