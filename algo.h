#ifndef ALGO_H
#define ALGO_H

/* Politica de substituicao usada pela TLB (16 entradas) e pela RAM (128 quadros). */
typedef enum {
    ALG_FIFO,
    ALG_LRU
} ReplacementAlg;

/* FIFO: escolhe o indice com o MENOR arrival_time (entrou ha mais tempo).
 * Retorna -1 se size <= 0. Em empate, devolve o menor indice. */
int select_victim_fifo(const unsigned long arrival_times[], int size);

/* LRU: escolhe o indice com o MENOR last_used (usado ha mais tempo).
 * Retorna -1 se size <= 0. Em empate, devolve o menor indice. */
int select_victim_lru(const unsigned long last_used_times[], int size);

/* Despachante: chama FIFO ou LRU conforme 'alg'. Funciona para qualquer
 * tamanho de estrutura (TLB ou RAM), pois so recebe vetores de tempos.
 * Retorna -1 se 'alg' for invalido ou size <= 0. */
int select_victim_by_algorithm(ReplacementAlg alg,
                               const unsigned long arrival_times[],
                               const unsigned long last_used_times[],
                               int size);

#endif 