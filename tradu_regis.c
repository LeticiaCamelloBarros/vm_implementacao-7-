#include "tradu_regis.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VIRTUAL_ADDRESS 65535
#define LOG_FILE "evidencias.log"

int translate_address(MemoryManager *mm, int virtual_address,
                      ReplacementAlg page_alg, ReplacementAlg tlb_alg,
                      FILE *backing_store, FILE *correct_fp,
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
    int tlb_index = -1;

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
        /* 4) Registra a traducao na TLB para os proximos acessos. */
        update_tlb(mm->tlb, page_num, frame_num, tlb_alg, *g_clock);
    }

    /* Atualiza as marcas de uso (necessarias para o LRU da RAM). */
    mm->frame_last_used[frame_num] = *g_clock;
    mm->page_table[page_num].last_used = *g_clock;

    /* Endereco fisico = quadro * 256 + deslocamento; le o byte da RAM. */
    int physical_address = frame_num * PAGE_SIZE + offset;
    int value = mm->physical_memory[physical_address];

    fprintf(correct_fp, "Virtual address: %d Physical address: %d Value: %d\n",
            virtual_address, physical_address, value);
    return 1;
}

static double ratio(int part, int total)
{
    return total > 0 ? (double)part / total : 0.0;
}

void write_correct_summary(const MemoryManager *mm, FILE *correct_fp)
{
    fprintf(correct_fp, "Number of Translated Addresses = %d\n", mm->total_addresses);
    fprintf(correct_fp, "Page Faults = %d\n", mm->page_faults);
    fprintf(correct_fp, "Page Fault Rate = %.3f\n",
            ratio(mm->page_faults, mm->total_addresses));
    fprintf(correct_fp, "TLB Hits = %d\n", mm->tlb_hits);
    fprintf(correct_fp, "TLB Hit Rate = %.3f\n",
            ratio(mm->tlb_hits, mm->total_addresses));
}

/* Imprime o relatorio em qualquer FILE* (log ou terminal). */
static void print_report(FILE *out, const MemoryManager *mm,
                         const char *addr_file, const char *page_alg_name,
                         const char *tlb_alg_name, const char *when)
{
    fprintf(out, "==== Simulador de Memoria Virtual - Evidencias ====\n");
    fprintf(out, "Data/hora: %s\n", when);
    fprintf(out, "Arquivo de enderecos: %s\n", addr_file);
    fprintf(out, "Algoritmo de paginas: %s\n", page_alg_name);
    fprintf(out, "Algoritmo da TLB: %s\n", tlb_alg_name);
    fprintf(out, "Enderecos lidos: %d\n", mm->total_addresses);
    fprintf(out, "Page Faults: %d (%.2f%%)\n", mm->page_faults,
            100.0 * ratio(mm->page_faults, mm->total_addresses));
    fprintf(out, "TLB Hits: %d (%.2f%%)\n", mm->tlb_hits,
            100.0 * ratio(mm->tlb_hits, mm->total_addresses));
    fprintf(out, "===================================================\n");
}

void write_final_report(const MemoryManager *mm, const char *addr_file,
                        const char *page_alg_name, const char *tlb_alg_name)
{
    char when[64];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    if (tm_now == NULL || strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", tm_now) == 0)
        snprintf(when, sizeof(when), "desconhecida");

    /* Modo append: nao apaga o historico gravado por 'script -a evidencias.log'. */
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", LOG_FILE);
    } else {
        print_report(log, mm, addr_file, page_alg_name, tlb_alg_name, when);
        fprintf(log, "\n");
        fclose(log);
    }

    printf("Simulacao concluida com sucesso!\n");
    print_report(stdout, mm, addr_file, page_alg_name, tlb_alg_name, when);
}