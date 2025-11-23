// detective_quest.c
// Desafio Detective Quest — Nível MESTRE
// (Inclui Novato: Árvore Binária de Salas, Aventureiro: BST de Pistas,
//  e Mestre: Tabela Hash de Suspeitos ⇄ Pistas)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

// =====================================================================
// Utilidades
// =====================================================================
static char *strdup_safe(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

// lower para hashing
static unsigned char tolower_u(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return (unsigned char)(c - 'A' + 'a');
    return c;
}

// =====================================================================
// Nível Novato — Árvore Binária de Salas
// =====================================================================
typedef struct Sala {
    char nome[48];
    struct Sala *esq;   // esquerda
    struct Sala *dir;   // direita
} Sala;

Sala* criarSala(const char *nome) {
    Sala *s = (Sala*)malloc(sizeof(Sala));
    if (!s) return NULL;
    snprintf(s->nome, sizeof(s->nome), "%s", nome);
    s->esq = s->dir = NULL;
    return s;
}

void conectarSalas(Sala *orig, Sala *esq, Sala *dir) {
    if (!orig) return;
    orig->esq = esq;
    orig->dir = dir;
}

// =====================================================================
// Nível Aventureiro — BST de Pistas
// =====================================================================
typedef struct PistaNode {
    char *texto;
    struct PistaNode *esq, *dir;
} PistaNode;

int cmp_str(const char *a, const char *b) {
    return strcmp(a, b);
}

bool bst_contem(PistaNode *raiz, const char *texto) {
    while (raiz) {
        int c = cmp_str(texto, raiz->texto);
        if (c == 0) return true;
        raiz = (c < 0) ? raiz->esq : raiz->dir;
    }
    return false;
}

PistaNode* bst_inserir(PistaNode *raiz, const char *texto) {
    if (!raiz) {
        PistaNode *n = (PistaNode*)malloc(sizeof(PistaNode));
        n->texto = strdup_safe(texto);
        n->esq = n->dir = NULL;
        return n;
    }
    int c = cmp_str(texto, raiz->texto);
    if (c < 0) raiz->esq = bst_inserir(raiz->esq, texto);
    else if (c > 0) raiz->dir = bst_inserir(raiz->dir, texto);
    // se igual, não insere duplicado
    return raiz;
}

void bst_em_ordem(PistaNode *raiz) {
    if (!raiz) return;
    bst_em_ordem(raiz->esq);
    printf(" - %s\n", raiz->texto);
    bst_em_ordem(raiz->dir);
}

void bst_destruir(PistaNode *raiz) {
    if (!raiz) return;
    bst_destruir(raiz->esq);
    bst_destruir(raiz->dir);
    free(raiz->texto);
    free(raiz);
}

// =====================================================================
// Nível Mestre — Hash de Suspeitos e lista de pistas por suspeito
// =====================================================================
typedef struct ClueNode {
    char *texto;
    struct ClueNode *next;
} ClueNode;

typedef struct Suspeito {
    char *nome;
    int  contagem;        // quantas pistas associadas (sem duplicar a mesma pista)
    ClueNode *pistas;     // lista encadeada de pistas
    struct Suspeito *next; // encadeamento para colisões (separate chaining)
} Suspeito;

#define HASH_TAM 23

typedef struct {
    Suspeito *buckets[HASH_TAM];
} HashSuspeitos;

unsigned hash_nome(const char *nome) {
    unsigned long sum = 0;
    for (const unsigned char *p = (const unsigned char*)nome; *p; ++p)
        sum += (unsigned long)tolower_u(*p);
    return (unsigned)(sum % HASH_TAM);
}

void inicializarHash(HashSuspeitos *h) {
    for (int i = 0; i < HASH_TAM; i++) h->buckets[i] = NULL;
}

Suspeito* buscarOuCriarSuspeito(HashSuspeitos *h, const char *nome) {
    unsigned idx = hash_nome(nome);
    Suspeito *cur = h->buckets[idx];
    while (cur) {
        if (strcmp(cur->nome, nome) == 0) return cur;
        cur = cur->next;
    }
    // criar
    Suspeito *novo = (Suspeito*)malloc(sizeof(Suspeito));
    novo->nome = strdup_safe(nome);
    novo->contagem = 0;
    novo->pistas = NULL;
    novo->next = h->buckets[idx];
    h->buckets[idx] = novo;
    return novo;
}

static bool suspeito_tem_pista(Suspeito *s, const char *pista) {
    for (ClueNode *c = s->pistas; c; c = c->next)
        if (strcmp(c->texto, pista) == 0) return true;
    return false;
}

// Registrar relação pista -> suspeito (sem duplicar a mesma pista no suspeito)
void inserirHash(HashSuspeitos *h, const char *pista, const char *suspeitoNome) {
    Suspeito *s = buscarOuCriarSuspeito(h, suspeitoNome);
    if (!suspeito_tem_pista(s, pista)) {
        ClueNode *n = (ClueNode*)malloc(sizeof(ClueNode));
        n->texto = strdup_safe(pista);
        n->next = s->pistas;
        s->pistas = n;
        s->contagem += 1;
    }
}

void listarAssociacoes(HashSuspeitos *h) {
    printf("\n=== Suspeitos & Pistas Relacionadas ===\n");
    for (int i = 0; i < HASH_TAM; i++) {
        for (Suspeito *s = h->buckets[i]; s; s = s->next) {
            printf("Suspeito: %s  | Pistas (%d):\n", s->nome, s->contagem);
            for (ClueNode *c = s->pistas; c; c = c->next) {
                printf("   - %s\n", c->texto);
            }
        }
    }
}

Suspeito* suspeitoMaisCitado(HashSuspeitos *h) {
    Suspeito *melhor = NULL;
    for (int i = 0; i < HASH_TAM; i++) {
        for (Suspeito *s = h->buckets[i]; s; s = s->next) {
            if (!melhor || s->contagem > melhor->contagem) melhor = s;
        }
    }
    return melhor;
}

void destruirHash(HashSuspeitos *h) {
    for (int i = 0; i < HASH_TAM; i++) {
        Suspeito *s = h->buckets[i];
        while (s) {
            Suspeito *nx = s->next;
            ClueNode *c = s->pistas;
            while (c) {
                ClueNode *cx = c->next;
                free(c->texto);
                free(c);
                c = cx;
            }
            free(s->nome);
            free(s);
            s = nx;
        }
        h->buckets[i] = NULL;
    }
}

// =====================================================================
// Integração: ao visitar salas, coletar pistas e relacionar com suspeitos
// =====================================================================

// Tabela de “roteiro” (sala → pista → suspeito)
typedef struct {
    const char *sala;
    const char *pista;
    const char *suspeito;
} ClueMap;

static const ClueMap ROTEIRO[] = {
    {"Biblioteca", "Livro rasgado com assinatura", "Professor"},
    {"Cozinha",    "Faca com marcas recentes",     "Chef"},
    {"Sotao",      "Pegadas cobertas de poeira",   "Jardineiro"},
    {"Escritorio", "Cofre arrombado",              "Mordomo"},
    {"Quarto",     "Perfume raro espalhado",       "Madame"},
    {"Porão",      "Luvas com fuligem",            "Jardineiro"},
    {"Jardim",     "Terra fresca nas botas",       "Jardineiro"},
};

static const size_t N_ROTEIRO = sizeof(ROTEIRO)/sizeof(ROTEIRO[0]);

// Coleta: insere pista na BST (se ainda não existe) e relaciona ao suspeito
void coletarPistaSeHouver(const char *nomeSala, PistaNode **pBST, HashSuspeitos *hash) {
    for (size_t i = 0; i < N_ROTEIRO; i++) {
        if (strcmp(ROTEIRO[i].sala, nomeSala) == 0) {
            // se a pista ainda não foi coletada, inserir na BST e na hash
            if (!bst_contem(*pBST, ROTEIRO[i].pista)) {
                *pBST = bst_inserir(*pBST, ROTEIRO[i].pista);
                inserirHash(hash, ROTEIRO[i].pista, ROTEIRO[i].suspeito);
                printf(">> Você encontrou uma PISTA em %s: \"%s\" (associada a %s)\n",
                       nomeSala, ROTEIRO[i].pista, ROTEIRO[i].suspeito);
            } else {
                printf(">> Você revisitou %s. A pista \"%s\" já foi registrada.\n",
                       nomeSala, ROTEIRO[i].pista);
            }
            return; // supondo 1 pista por sala nessa tabela
        }
    }
    // sala sem pista no roteiro — nada acontece
}

// =====================================================================
// Construção fixa do mapa da mansão (árvore binária)
// =====================================================================
// Layout exemplo:
//
//                    [Hall de Entrada]
//                   /                 \
//             [Biblioteca]         [Cozinha]
//              /        \           /      \
//         [Sotao]   [Escritorio] [Quarto] [Jardim]
//            \                          /
//           [Porao]                 [Adega]
//
// Algumas salas não têm ambos os filhos; é proposital para navegação.

Sala* construirMansao(void) {
    Sala *hall       = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *cozinha    = criarSala("Cozinha");
    Sala *sotao      = criarSala("Sotao");
    Sala *escritorio = criarSala("Escritorio");
    Sala *quarto     = criarSala("Quarto");
    Sala *jardim     = criarSala("Jardim");
    Sala *porao      = criarSala("Porão");
    Sala *adega      = criarSala("Adega");

    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, sotao, escritorio);
    conectarSalas(cozinha, quarto, jardim);
    conectarSalas(sotao, NULL, porao);        // só direita leva ao Porão
    conectarSalas(jardim, adega, NULL);       // só esquerda leva à Adega

    return hall; // raiz da árvore
}

void destruirArvoreSalas(Sala *r) {
    if (!r) return;
    destruirArvoreSalas(r->esq);
    destruirArvoreSalas(r->dir);
    free(r);
}

// =====================================================================
// Exploração (Novato) + Coleta automática de pistas (Aventureiro/Mestre)
// =====================================================================
void explorarSalas(Sala *raiz, PistaNode **pBST, HashSuspeitos *hash) {
    if (!raiz) return;
    Sala *atual = raiz;

    puts("\n=== Exploração da Mansão ===");
    puts("Use: (e) esquerda, (d) direita, (s) sair da exploração.\n");

    while (1) {
        printf("Você está em: ** %s **\n", atual->nome);
        // Ao chegar em uma sala, tenta coletar pista (se houver para essa sala)
        coletarPistaSeHouver(atual->nome, pBST, hash);

        printf("Caminhos: ");
        if (atual->esq) printf("[e] %s  ", atual->esq->nome);
        if (atual->dir) printf("[d] %s  ", atual->dir->nome);
        printf("[s] sair\n");

        printf("Escolha: ");
        char op = 0;
        if (scanf(" %c", &op) != 1) {
            // limpar stdin
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Entrada inválida.");
            continue;
        }
        if (op == 's' || op == 'S') {
            puts("Encerrando exploração.\n");
            break;
        } else if ((op == 'e' || op == 'E') && atual->esq) {
            atual = atual->esq;
        } else if ((op == 'd' || op == 'D') && atual->dir) {
            atual = atual->dir;
        } else {
            puts("Movimento inválido neste ponto do mapa.");
        }
    }
}

// =====================================================================
// UI principal
// =====================================================================
static void mostrarMenu(void) {
    puts("=== DETECTIVE QUEST ===");
    puts("1 - Explorar mansão");
    puts("2 - Listar pistas (em ordem alfabética)");
    puts("3 - Listar suspeitos e pistas associadas");
    puts("4 - Mostrar suspeito mais provável");
    puts("0 - Sair");
    printf("Escolha: ");
}

int main(void) {
    // Semente só pra dar um charme, embora não haja aleatoriedade aqui
    srand((unsigned)time(NULL));

    // Construir mansão (árvore binária fixa)
    Sala *mansao = construirMansao();

    // Estruturas de investigação
    PistaNode *pistasBST = NULL;   // BST de pistas (strings)
    HashSuspeitos hash;            // Hash: suspeito -> lista de pistas
    inicializarHash(&hash);

    int op;
    puts("Bem-vindo ao Desafio Detective Quest!");
    while (1) {
        mostrarMenu();
        if (scanf("%d", &op) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Entrada inválida.");
            continue;
        }
        puts("------------------------------------------------------------");
        switch (op) {
            case 1:
                explorarSalas(mansao, &pistasBST, &hash);
                break;
            case 2:
                if (!pistasBST) puts("Nenhuma pista coletada ainda.");
                else {
                    puts("Pistas (ordem alfabética):");
                    bst_em_ordem(pistasBST);
                }
                break;
            case 3:
                listarAssociacoes(&hash);
                break;
            case 4: {
                Suspeito *s = suspeitoMaisCitado(&hash);
                if (!s) puts("Ainda não há suspeitos relacionados.");
                else {
                    printf("Suspeito mais provável: %s (%d pista%s)\n",
                           s->nome, s->contagem, s->contagem==1?"":"s");
                }
                break;
            }
            case 0:
                puts("Até a próxima investigação!");
                destruirArvoreSalas(mansao);
                bst_destruir(pistasBST);
                destruirHash(&hash);
                return 0;
            default:
                puts("Opção inexistente.");
        }
        puts("------------------------------------------------------------\n");
    }
}