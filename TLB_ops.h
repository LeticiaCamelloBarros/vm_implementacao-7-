#ifndef TLB_OPS_H
#define TLB_OPS_H

#include "busca.h"  /* TLBEntry e TLB_SIZE */
#include "algo.h"   /* ReplacementAlg */

/* Registra a traducao page_num -> frame_num na TLB apos um TLB Miss.
 * Usa um slot livre, se houver; senao, substitui a vitima escolhida
 * por tlb_alg (FIFO ou LRU). g_clock e o relogio logico atual. */
void update_tlb(TLBEntry tlb[], int page_num, int frame_num,
                ReplacementAlg tlb_alg, unsigned long g_clock);

/* Invalida a entrada da TLB que guarda page_num (se existir).
 * Chamada quando a pagina e removida da RAM, para manter coerencia. */
void invalidate_tlb_page(TLBEntry tlb[], int page_num);

#endif 