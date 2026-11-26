# Simulador de Gerenciador de Memória Virtual em C

**Disciplina:** Infraestrutura de Software — CESAR School
**Base:** *Designing a Virtual Memory Manager* (Silberschatz et al., *Operating System Concepts*, 10ª ed., p. P-51), com as modificações do enunciado.

## O que o programa faz

Traduz endereços lógicos de 16 bits em endereços físicos e lê o byte correspondente.

| Item | Valor |
| :--- | :--- |
| Endereço lógico | 16 bits: página = `end >> 8`, deslocamento = `end & 0xFF` |
| Página / quadro | 256 bytes |
| Espaço lógico | 256 páginas (64 KB) |
| Memória física | **128 quadros** (32 KB), preenchidos de 0 a 127 |
| TLB | **16 entradas**, preenchidas de 0 a 15 |
| Busca na TLB | **16 `pthreads`**, uma por entrada |
| Memória secundária | `BACKING_STORE.bin` (lido na pasta onde o programa roda) |
| Substituição de página | `fifo` ou `lru` |
| Substituição da TLB | `fifo` ou `lru` |

Regras implementadas:

- Com a RAM cheia, o algoritmo escolhe o quadro vítima.
- Quando uma página sai da RAM, a entrada dela na TLB é invalidada. A posição liberada é a **primeira** preenchida no próximo TLB miss, antes de qualquer FIFO/LRU.
- No `lru`, todo acesso conta como uso, inclusive o que acerta a TLB.

## Compilar

```bash
make clean
make
```

Gera o executável `vm` na raiz. Requer `gcc` e `pthreads` (Linux, Unix ou macOS).

## Executar

```bash
./vm <arquivo_enderecos> <fifo|lru> <fifo|lru>
```

1º argumento: arquivo de endereços (um inteiro de 0 a 65535 por linha). 2º: algoritmo de página. 3º: algoritmo da TLB.

```bash
./vm addresses.txt lru fifo     # lru na página, fifo na TLB
```

O `BACKING_STORE.bin` e o arquivo de endereços são fornecidos pelo professor e **não** fazem parte do repositório.

## Saída

Uma linha por endereço, na ordem do arquivo, na **saída padrão**:

```
Virtual address: 16916 TLB: 0 Physical address: 20 Value: 0
```

- `TLB` é o **índice (0 a 15) da entrada da TLB** usada: onde a página foi encontrada (hit) ou onde a tradução foi gravada (miss).
- `Value` é o byte lido (`signed char`, de -128 a 127).
- Nenhuma estatística, cabeçalho ou mensagem extra é impressa em `stdout`.

## Tratamento de erro

Mensagem em `stderr` e código de saída diferente de zero (1) quando:

- o número de argumentos não é 3;
- o algoritmo de página ou da TLB não é `fifo` nem `lru`;
- o arquivo de endereços não existe ou não pode ser aberto;
- o arquivo tem linha que não é um inteiro de 0 a 65535, linha longa demais ou nenhum endereço;
- o `BACKING_STORE.bin` não pode ser aberto.

O arquivo de endereços é validado **inteiro antes** da simulação, então nenhuma saída parcial é impressa quando há erro de formatação.

```bash
./vm; echo $?                              # uso incorreto
./vm addresses.txt abc lru; echo $?        # algoritmo invalido
./vm naoexiste.txt lru fifo; echo $?       # arquivo inexistente
```

## Arquitetura

| Arquivo | Responsabilidade |
| :--- | :--- |
| `main.c` | Argumentos, validação do arquivo de endereços, inicialização e laço principal. |
| `tradu_regis.c/.h` | Estado da simulação (`MemoryManager`) e tradução de um endereço, com impressão da linha de saída. |
| `busca.c/.h` | Busca paralela na TLB: 16 threads, `pthread_create` + `pthread_join`. |
| `TLB_ops.c/.h` | `update_tlb` (slot livre primeiro, senão vítima FIFO/LRU; retorna o índice) e `invalidate_tlb_page`. |
| `MF_PD.c/.h` | RAM física, paginação por demanda e tratamento de page fault. |
| `algo.c/.h` | Seleção de vítima FIFO e LRU, usada pela TLB e pela RAM. |
| `Makefile` | Alvos `all`, `clean` e `run`. |

### Decisões técnicas

1. **Busca na TLB sem mutex.** Cada thread escreve só na sua própria struct e a TLB é somente lida durante a busca, então não há condição de corrida.
2. **Algoritmos como funções puras.** `algo.c` recebe só vetores de tempos, então o mesmo código serve para a TLB (16) e para a RAM (128).
3. **Relógio lógico global.** Cada acesso incrementa um contador usado como instante de chegada (FIFO) e de último uso (LRU).
4. **Validação prévia do arquivo.** Evita saída parcial em caso de erro de formatação.

## Estrutura do repositório

Código-fonte (`*.c`, `*.h`), `Makefile`, `README.md`, relatório `<login>.pdf` e `evidencias.log`.
