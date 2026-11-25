/*
 * main.c - Simulador de Gerenciador de Memoria Virtual
 * Uso: ./vm <arquivo_enderecos> <algoritmo_pagina: fifo|lru> <algoritmo_tlb: fifo|lru>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "tradu_regis.h"

#define BACKING_STORE_FILE "BACKING_STORE.bin"
#define OUTPUT_FILE        "correct.txt"

static MemoryManager mm;   /* estado da simulacao (zerado por ser static) */

int main(int argc, char *argv[])
{
    /* ---- Validacao dos argumentos ---- */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_enderecos> <fifo|lru> <fifo|lru>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ReplacementAlg page_alg, tlb_alg;

    if (strcasecmp(argv[2], "fifo") == 0) {
        page_alg = ALG_FIFO;
    } else if (strcasecmp(argv[2], "lru") == 0) {
        page_alg = ALG_LRU;
    } else {
        fprintf(stderr, "Erro: algoritmo de paginas invalido '%s' (use fifo ou lru)\n", argv[2]);
        return EXIT_FAILURE;
    }

    if (strcasecmp(argv[3], "fifo") == 0) {
        tlb_alg = ALG_FIFO;
    } else if (strcasecmp(argv[3], "lru") == 0) {
        tlb_alg = ALG_LRU;
    } else {
        fprintf(stderr, "Erro: algoritmo da TLB invalido '%s' (use fifo ou lru)\n", argv[3]);
        return EXIT_FAILURE;
    }

    /* ---- Inicializacao das estruturas ---- */
    for (int i = 0; i < TLB_SIZE; i++) {
        mm.tlb[i].page_num = -1;
        mm.tlb[i].frame_num = -1;
        mm.tlb[i].valid = 0;
        mm.tlb[i].arrival_time = 0;
        mm.tlb[i].last_used = 0;
    }
    for (int i = 0; i < NUM_PAGES; i++) {
        mm.page_table[i].frame_num = -1;
        mm.page_table[i].valid = 0;
        mm.page_table[i].arrival_time = 0;
        mm.page_table[i].last_used = 0;
    }
    for (int i = 0; i < NUM_FRAMES; i++) {
        mm.frame_to_page[i] = -1;
        mm.frame_arrival[i] = 0;
        mm.frame_last_used[i] = 0;
    }
    memset(mm.physical_memory, 0, sizeof(mm.physical_memory));
    mm.allocated_frames = 0;
    mm.total_addresses = 0;
    mm.page_faults = 0;
    mm.tlb_hits = 0;

    unsigned long g_clock = 0;

    /* ---- Abertura dos arquivos ---- */
    FILE *addr_fp = fopen(argv[1], "r");
    if (addr_fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    FILE *backing_store = fopen(BACKING_STORE_FILE, "rb");
    if (backing_store == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", BACKING_STORE_FILE);
        fclose(addr_fp);
        return EXIT_FAILURE;
    }

    FILE *correct_fp = fopen(OUTPUT_FILE, "w");
    if (correct_fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'\n", OUTPUT_FILE);
        fclose(addr_fp);
        fclose(backing_store);
        return EXIT_FAILURE;
    }

    /* ---- Loop principal: um endereco por linha ---- */
    int vaddr;
    int status = EXIT_SUCCESS;

    while (fscanf(addr_fp, "%d", &vaddr) == 1) {
        if (!translate_address(&mm, vaddr, page_alg, tlb_alg, backing_store,
                               correct_fp, &g_clock)) {
            status = EXIT_FAILURE;
            break;
        }
    }

    write_correct_summary(&mm, correct_fp);

    fclose(addr_fp);
    fclose(backing_store);
    fclose(correct_fp);

    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Simulacao interrompida por erro.\n");
        return status;
    }

    write_final_report(&mm, argv[1], argv[2], argv[3]);
    return EXIT_SUCCESS;
}