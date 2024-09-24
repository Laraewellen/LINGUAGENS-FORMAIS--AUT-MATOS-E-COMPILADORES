#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TAMANHO_MAX_LEXEMA 100
#define TAMANHO_TABELA 100

typedef struct {
    char tipo[TAMANHO_MAX_LEXEMA];
    char valor[TAMANHO_MAX_LEXEMA];
    int linha;
    int coluna;
} Token;

typedef struct {
    char lexema[TAMANHO_MAX_LEXEMA];
    char tipo[TAMANHO_MAX_LEXEMA];
} Simbolo;

Simbolo tabela_simbolos[TAMANHO_TABELA];
int contador_simbolos = 0;

int linha = 1, coluna = 0;

const char *palavras_chave[] = {
    "program", "var", "integer", "real", "begin", "end",
    "if", "then", "else", "while", "do", "write", "read", NULL
};

void adicionar_tabela_simbolos(const char *lexema, const char *tipo) {
    for (int i = 0; i < contador_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].lexema, lexema) == 0) {
            return;
        }
    }
    strcpy(tabela_simbolos[contador_simbolos].lexema, lexema);
    strcpy(tabela_simbolos[contador_simbolos].tipo, tipo);
    contador_simbolos++;
}

int e_palavra_chave(const char *lexema) {
    for (int i = 0; palavras_chave[i] != NULL; i++) {
        if (!strcmp(lexema, palavras_chave[i])) return 1;
    }
    return 0;
}

void exibir_erro_lexico() {
    printf("Erro léxico na linha %d, coluna %d\n", linha, coluna);
}

Token proximo_token(FILE *arquivo) {
    Token token;
    char c;
    int i = 0;

    while ((c = fgetc(arquivo)) != EOF) {
        coluna++;
        if (isspace(c)) {
            if (c == '\n') { linha++; coluna = 0; }
            continue;
        }

        if (isalpha(c)) {
            token.valor[i++] = c;
            while (isalnum(c = fgetc(arquivo))) token.valor[i++] = c, coluna++;
            ungetc(c, arquivo);
            token.valor[i] = '\0';
            if (e_palavra_chave(token.valor)) {
                strcpy(token.tipo, "RESERVADA");
            } else {
                strcpy(token.tipo, "IDENTIFICADOR");
                adicionar_tabela_simbolos(token.valor, "IDENTIFICADOR");
            }
        } else if (isdigit(c)) {
            token.valor[i++] = c;
            while (isdigit(c = fgetc(arquivo))) token.valor[i++] = c, coluna++;
            ungetc(c, arquivo);
            token.valor[i] = '\0';
            strcpy(token.tipo, "NUMERO");
        } else {
            token.valor[0] = c;
            token.valor[1] = '\0';
            switch (c) {
                case '+': strcpy(token.tipo, "OP_AD"); break;
                case '-': strcpy(token.tipo, "OP_MIN"); break;
                case '*': strcpy(token.tipo, "OP_MUL"); break;
                case '/': strcpy(token.tipo, "OP_DIV"); break;
                case '=': strcpy(token.tipo, "OP_EQ"); break;
                case '>': 
                    if ((c = fgetc(arquivo)) == '=') {
                        strcpy(token.tipo, "OP_GE");
                        token.valor[1] = '=';
                        token.valor[2] = '\0';
                        coluna++;
                    } else {
                        ungetc(c, arquivo);
                        strcpy(token.tipo, "OP_GT");
                    }
                    break;
                case '<': 
                    if ((c = fgetc(arquivo)) == '=') {
                        strcpy(token.tipo, "OP_LE");
                        token.valor[1] = '=';
                        token.valor[2] = '\0';
                        coluna++;
                    } else if (c == '>') {
                        strcpy(token.tipo, "OP_NE");
                        token.valor[1] = '>';
                        token.valor[2] = '\0';
                    } else {
                        ungetc(c, arquivo);
                        strcpy(token.tipo, "OP_LT");
                    }
                    break;
                case ';': strcpy(token.tipo, "SMB_SEM"); break;
                case ',': strcpy(token.tipo, "SMB_COM"); break;
                case '(': strcpy(token.tipo, "SMB_OPA"); break;
                case ')': strcpy(token.tipo, "SMB_CPA"); break;
                case '{': strcpy(token.tipo, "SMB_OBC"); break;
                case '}': strcpy(token.tipo, "SMB_CBC"); break;
                default: 
                    exibir_erro_lexico(); 
                    continue;
            }
        }
        token.linha = linha;
        token.coluna = coluna;
        return token;
    }

    strcpy(token.tipo, "EOF");
    token.valor[0] = '\0';
    token.linha = linha;
    token.coluna = coluna;
    return token;
}

void realizar_analise(FILE *arquivo, FILE *saida) {
    Token token;
    do {
        token = proximo_token(arquivo);
        if (strcmp(token.tipo, "EOF") != 0) {
            fprintf(saida, "<%s, %s> na linha %d, coluna %d\n", token.tipo, token.valor, token.linha, token.coluna);
        }
    } while (strcmp(token.tipo, "EOF") != 0);

    fprintf(saida, "\nTabela de Símbolos:\n");
    for (int i = 0; i < contador_simbolos; i++) {
        fprintf(saida, "<%s, %s>\n", tabela_simbolos[i].tipo, tabela_simbolos[i].lexema);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo de entrada>\n", argv[0]);
        return 1;
    }
    FILE *entrada = fopen(argv[1], "r");
    if (!entrada) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    char nome_saida[256];
    snprintf(nome_saida, sizeof(nome_saida), "%s.lex", argv[1]);
    FILE *saida = fopen(nome_saida, "w");
    if (!saida) {
        perror("Erro ao criar arquivo de saída");
        fclose(entrada);
        return 1;
    }

    realizar_analise(entrada, saida);
    fclose(entrada);
    fclose(saida);

    return 0;
}
