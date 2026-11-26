#include "tradu_regis.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_VIRTUAL_ADDRESS 65535

int translate_address(MemoryManager *mm, int virtual_address,
                      ReplacementAlg page_alg, ReplacementAlg tlb_alg,
                      FILE *backing_store, FILE *out,
                      unsigned long *g_clock)
{
    if (virtual_address < 0 || virtual_address > MAX_VIRTUAL_ADDRESS) {
        fprintf(stderr, "Erro: endereco virtual fora do intervalo: %d\n",
                virtual_address);
        return 0;
    }

    /* Separa o endereco de 16 bits: [ pagina (8 bits) | deslocamento (8 bits) ] */
    int page_num = (virtual_address >> 8) & 0xFF;
    int offset = virtual_address & 0xFF;
    int frame_num = -1;
    int tlb_index = -1;      /* indice (0..15) da entrada da TLB usada */

    (*g_clock)++;            /* cada acesso avanca o relogio logico */
    mm->total_addresses++;

    /* 1) Procura a pagina na TLB (busca paralela com 16 threads). */
    if (search_tlb_parallel(page_num, mm->tlb, &frame_num, &tlb_index)) {
        mm->tlb_hits++;                          /* TLB Hit */
        mm->tlb[tlb_index].last_used = *g_clock; /* LRU da TLB */
    } else {
        /* 2) TLB Miss: consulta a Tabela de Paginas. */
        if (mm->page_table[page_num].valid) {
            frame_num = mm->page_table[page_num].frame_num;
        } else {
            /* 3) Page Fault: traz a pagina do disco (aloca ou despeja). */
            frame_num = handle_page_fault(mm->page_table, mm->physical_memory,
                                          mm->frame_to_page, mm->frame_arrival,
                                          mm->frame_last_used,
                                          &mm->allocated_frames,
                                          &mm->page_faults, mm->tlb, page_num,
                                          page_alg, backing_store, *g_clock);
            if (frame_num < 0)
                return 0;
        }
        /* 4) Registra a traducao na TLB e guarda o indice onde foi gravada
         *    (ele e impresso no campo "TLB:" da linha de saida). */
        tlb_index = update_tlb(mm->tlb, page_num, frame_num, tlb_alg, *g_clock);
        if (tlb_index < 0)
            return 0;
    }

    /* Atualiza as marcas de uso (necessarias para o LRU da RAM). */
    mm->frame_last_used[frame_num] = *g_clock;
    mm->page_table[page_num].last_used = *g_clock;

    /* Endereco fisico = quadro * 256 + deslocamento; le o byte da RAM. */
    int physical_address = frame_num * PAGE_SIZE + offset;
    int value = mm->physical_memory[physical_address];

    fprintf(out,
            "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
            virtual_address, tlb_index, physical_address, value);
    return 1;
}
