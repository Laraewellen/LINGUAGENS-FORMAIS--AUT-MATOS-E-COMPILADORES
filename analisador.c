#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TABELA 100

typedef enum {
    Identificador, Numero, PalavraChave, OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, OP_ASS, OP_LT, OP_MIN,
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA, StringNaoFechada, CaracterDesconhecido
} TokenType;

typedef struct {
    TokenType tipo;
    char valor[100];
    int linha, coluna;
} Token;

typedef struct {
    char lexema[100];
    TokenType tipo;
} Simbolo;

Simbolo tabelaSimbolos[MAX_TABELA];
int tamanhoTabelaSimbolos = 0;

const char *palavrasChave[] = {"if", "then", "else", "begin", "end", "program", "var", "integer", "real", "while"};
int numPalavrasChave = 10;

void imprimeCaractere(char c, int linha, int coluna) {
    printf("Lendo caractere: %c (linha %d, coluna %d)\n", c == '\n' ? ' ' : c, linha, coluna);
}

int letra(char c) {
    return isalpha(c);
}

int digito(char c) {
    return isdigit(c);
}

TokenType identificaOperador(char *buffer) {
    if (strcmp(buffer, "=") == 0) return OP_EQ;
    if (strcmp(buffer, ">=") == 0) return OP_GE;
    if (strcmp(buffer, "*") == 0) return OP_MUL;
    if (strcmp(buffer, "<>") == 0) return OP_NE;
    if (strcmp(buffer, "<=") == 0) return OP_LE;
    if (strcmp(buffer, "/") == 0) return OP_DIV;
    if (strcmp(buffer, ">") == 0) return OP_GT;
    if (strcmp(buffer, "+") == 0) return OP_AD;
    if (strcmp(buffer, ":=") == 0) return OP_ASS;
    if (strcmp(buffer, "<") == 0) return OP_LT;
    if (strcmp(buffer, "-") == 0) return OP_MIN;
    return CaracterDesconhecido;
}

int palavraChave(char *buffer) {
    for (int i = 0; i < numPalavrasChave; i++) {
        if (strcmp(palavrasChave[i], buffer) == 0) return 1;
    }
    return 0;
}

int consultaTabelaSimbolos(char *lexema) {
    for (int i = 0; i < tamanhoTabelaSimbolos; i++) {
        if (strcmp(tabelaSimbolos[i].lexema, lexema) == 0) return i;
    }
    return -1;
}

void adicionaTabelaSimbolos(char *lexema, TokenType tipo) {
    if (consultaTabelaSimbolos(lexema) == -1 && (tipo == Identificador || tipo == PalavraChave)) {
        strcpy(tabelaSimbolos[tamanhoTabelaSimbolos].lexema, lexema);
        tabelaSimbolos[tamanhoTabelaSimbolos].tipo = tipo;
        tamanhoTabelaSimbolos++;
    }
}

void salvaToken(FILE *arquivoLex, Token token) {
    fprintf(arquivoLex, "<%d, %s> Linha: %d, Coluna: %d\n", token.tipo, token.valor, token.linha, token.coluna);
}

void analisadorLexico(FILE *arquivoFonte, FILE *arquivoLex) {
    char c, buffer[100];
    int linha = 1, coluna = 0, i = 0;
    Token token;

    while ((c = fgetc(arquivoFonte)) != EOF) {
        coluna++;
        imprimeCaractere(c, linha, coluna);

        if (isspace(c)) {
            if (c == '\n') { 
                linha++; 
                coluna = 0;
            }
        } else if (c == '{') {
            while ((c = fgetc(arquivoFonte)) != '}' && c != EOF) {
                if (c == '\n') {
                    linha++;
                    coluna = 0;
                }
                coluna++;
            }
        } else if (letra(c)) {
            buffer[i++] = c;
            while ((c = fgetc(arquivoFonte)) != EOF && (letra(c) || digito(c))) {
                buffer[i++] = c;
                coluna++;
                imprimeCaractere(c, linha, coluna);
            }
            buffer[i] = '\0';
            fseek(arquivoFonte, -1, SEEK_CUR);
            coluna--;

            token.tipo = palavraChave(buffer) ? PalavraChave : Identificador;
            strcpy(token.valor, buffer);
            token.linha = linha;
            token.coluna = coluna - i + 1;
            salvaToken(arquivoLex, token);
            adicionaTabelaSimbolos(buffer, token.tipo);
            i = 0;
        } else if (digito(c)) {
            buffer[i++] = c;
            while ((c = fgetc(arquivoFonte)) != EOF && digito(c)) {
                buffer[i++] = c;
                coluna++;
                imprimeCaractere(c, linha, coluna);
            }
            buffer[i] = '\0';
            fseek(arquivoFonte, -1, SEEK_CUR);
            coluna--;

            token.tipo = Numero;
            strcpy(token.valor, buffer);
            token.linha = linha;
            token.coluna = coluna - i + 1;
            salvaToken(arquivoLex, token);
            i = 0;
        } else if (strchr("=<>+-*/{},;()", c)) {
            buffer[i++] = c;
            imprimeCaractere(c, linha, coluna);
            if (c == ':' || c == '<' || c == '>') {
                char prox = fgetc(arquivoFonte);
                if ((c == ':' && prox == '=') || (c == '<' && (prox == '>' || prox == '=')) || (c == '>' && prox == '=')) {
                    buffer[i++] = prox;
                    coluna++;
                    imprimeCaractere(prox, linha, coluna);
                } else {
                    fseek(arquivoFonte, -1, SEEK_CUR);
                    coluna--;
                }
            }
            buffer[i] = '\0';
            token.tipo = identificaOperador(buffer);
            strcpy(token.valor, buffer);
            token.linha = linha;
            token.coluna = coluna - i + 1;
            salvaToken(arquivoLex, token);
            i = 0;
        } else {
            token.tipo = CaracterDesconhecido;
            token.valor[0] = c;
            token.valor[1] = '\0';
            token.linha = linha;
            token.coluna = coluna;
            salvaToken(arquivoLex, token);
        }
    }
}

void imprimeTabelaSimbolos() {
    printf("\nTabela de Símbolos:\n");
    for (int i = 0; i < tamanhoTabelaSimbolos; i++) {
        printf("Lexema: %s, Tipo: %d\n", tabelaSimbolos[i].lexema, tabelaSimbolos[i].tipo);
    }
}

int main() {
    FILE *arquivoFonte = fopen("analisador.txt", "r");
    FILE *arquivoLex = fopen("tokens.lex", "w");

    if (!arquivoFonte) {
        printf("Erro ao abrir o arquivo fonte.\n");
        return 1;
    }

    analisadorLexico(arquivoFonte, arquivoLex);
    imprimeTabelaSimbolos();

    fclose(arquivoFonte);
    fclose(arquivoLex);
    return 0;
}
