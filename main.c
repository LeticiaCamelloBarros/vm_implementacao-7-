/*
 * main.c - Simulador de Gerenciador de Memoria Virtual
 * Uso: ./vm <arquivo_enderecos> <algoritmo_pagina: fifo|lru> <algoritmo_tlb: fifo|lru>
 *
 * Saida: somente as linhas de traducao, na saida padrao (stdout).
 * Erros: mensagem em stderr e codigo de saida diferente de zero.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "tradu_regis.h"

#define BACKING_STORE_FILE "BACKING_STORE.bin"
#define MAX_VIRTUAL_ADDRESS 65535
#define LINE_MAX_LEN 256

static MemoryManager mm;   /* estado da simulacao (zerado por ser static) */

static int parse_alg(const char *s, ReplacementAlg *out)
{
    if (strcasecmp(s, "fifo") == 0) { *out = ALG_FIFO; return 1; }
    if (strcasecmp(s, "lru") == 0)  { *out = ALG_LRU;  return 1; }
    return 0;
}

/* Interpreta uma linha do arquivo de enderecos.
 * Retorna 1 (endereco valido em *out), 0 (linha em branco) ou -1 (invalida). */
static int parse_line(const char *line, int *out)
{
    const char *p = line;
    char *end;
    long v;

    while (isspace((unsigned char)*p))
        p++;
    if (*p == '\0')
        return 0;

    errno = 0;
    v = strtol(p, &end, 10);
    if (end == p || errno != 0)
        return -1;
    while (isspace((unsigned char)*end))
        end++;
    if (*end != '\0')
        return -1;
    if (v < 0 || v > MAX_VIRTUAL_ADDRESS)
        return -1;

    *out = (int)v;
    return 1;
}

/* Le o arquivo inteiro antes de simular, para nao imprimir saida parcial
 * quando houver erro de formatacao. Retorna 1 se valido, 0 caso contrario. */
static int validate_address_file(FILE *fp, const char *name)
{
    char line[LINE_MAX_LEN];
    long lineno = 0;
    long count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        size_t len = strlen(line);
        int v;

        lineno++;
        if (len == sizeof(line) - 1 && line[len - 1] != '\n' && !feof(fp)) {
            fprintf(stderr, "Erro: linha %ld de '%s' longa demais\n", lineno, name);
            return 0;
        }
        int r = parse_line(line, &v);
        if (r < 0) {
            fprintf(stderr,
                    "Erro: linha %ld de '%s' invalida (esperado um inteiro de 0 a %d)\n",
                    lineno, name, MAX_VIRTUAL_ADDRESS);
            return 0;
        }
        if (r == 1)
            count++;
    }
    if (ferror(fp)) {
        fprintf(stderr, "Erro: falha ao ler '%s'\n", name);
        return 0;
    }
    if (count == 0) {
        fprintf(stderr, "Erro: '%s' nao contem enderecos\n", name);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    ReplacementAlg page_alg, tlb_alg;

    /* ---- Validacao dos argumentos ---- */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_enderecos> <fifo|lru> <fifo|lru>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!parse_alg(argv[2], &page_alg)) {
        fprintf(stderr, "Erro: algoritmo de paginas invalido '%s' (use fifo ou lru)\n", argv[2]);
        return EXIT_FAILURE;
    }
    if (!parse_alg(argv[3], &tlb_alg)) {
        fprintf(stderr, "Erro: algoritmo da TLB invalido '%s' (use fifo ou lru)\n", argv[3]);
        return EXIT_FAILURE;
    }

    /* ---- Abertura e validacao dos arquivos ---- */
    FILE *addr_fp = fopen(argv[1], "r");
    if (addr_fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (!validate_address_file(addr_fp, argv[1])) {
        fclose(addr_fp);
        return EXIT_FAILURE;
    }
    rewind(addr_fp);

    FILE *backing_store = fopen(BACKING_STORE_FILE, "rb");
    if (backing_store == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", BACKING_STORE_FILE);
        fclose(addr_fp);
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

    unsigned long g_clock = 0;
    int status = EXIT_SUCCESS;
    char line[LINE_MAX_LEN];

    /* ---- Loop principal: uma traducao por endereco, em stdout ---- */
    while (fgets(line, sizeof(line), addr_fp) != NULL) {
        int vaddr;
        int r = parse_line(line, &vaddr);

        if (r == 0)
            continue;
        if (r < 0 || !translate_address(&mm, vaddr, page_alg, tlb_alg,
                                        backing_store, stdout, &g_clock)) {
            status = EXIT_FAILURE;
            break;
        }
    }

    fclose(addr_fp);
    fclose(backing_store);

    if (status != EXIT_SUCCESS)
        fprintf(stderr, "Simulacao interrompida por erro.\n");
    return status;
}
