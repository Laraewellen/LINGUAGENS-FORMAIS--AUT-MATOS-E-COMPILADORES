#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TABELA 100

typedef enum {
    IDENTIFICADOR, NUMERO, PALAVRA_CHAVE, OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, OP_ASS, OP_LT, OP_MIN,
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA, STRING_NAO_FECHADA, CARACTERE_DESCONHECIDO
} TokenType;

typedef struct {
    TokenType tipo;
    char valor[100];
    int linha;
    int coluna;
} Token;

typedef struct {
    char nome[100];
    TokenType tipo;
} Simbolo;

const char *palavrasChave[] = {"if", "then", "else", "begin", "end", "program", "var", "integer", "real", "while"};
const int numPalavrasChave = 10;

Simbolo tabelaSimbolos[MAX_TABELA];
int tabelaSimbolosSize = 0;

char *codigo;
int i = 0, linha = 1, coluna = 1;
FILE *arquivoLex;

int ehLetra(char c) {
    return isalpha(c);
}

int ehDigito(char c) {
    return isdigit(c);
}

TokenType identificaOperadorSimbolo(const char *buffer) {
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
    if (strcmp(buffer, "{") == 0) return SMB_OBC;
    if (strcmp(buffer, ",") == 0) return SMB_COM;
    if (strcmp(buffer, "}") == 0) return SMB_CBC;
    if (strcmp(buffer, ";") == 0) return SMB_SEM;
    if (strcmp(buffer, "(") == 0) return SMB_OPA;
    if (strcmp(buffer, ")") == 0) return SMB_CPA;
    return CARACTERE_DESCONHECIDO;
}

int buscaNaTabelaSimbolos(const char *nome) {
    for (int j = 0; j < tabelaSimbolosSize; j++) {
        if (strcmp(tabelaSimbolos[j].nome, nome) == 0) {
            return 1;
        }
    }
    return 0;
}

void adicionaNaTabelaSimbolos(const char *nome, TokenType tipo) {
    if (!buscaNaTabelaSimbolos(nome)) {
        strcpy(tabelaSimbolos[tabelaSimbolosSize].nome, nome);
        tabelaSimbolos[tabelaSimbolosSize].tipo = tipo;
        tabelaSimbolosSize++;
    }
}

void salvaTokenEmArquivo(Token token) {
    fprintf(arquivoLex, "<%d, %s, %d, %d>\n", token.tipo, token.valor, token.linha, token.coluna);
}

void gerarRelatorio() {
    arquivoLex = fopen("tokens.lex", "w");
    if (arquivoLex == NULL) {
        perror("Erro ao abrir o arquivo tokens.lex");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < tabelaSimbolosSize; j++) {
        Token token;
        token.tipo = tabelaSimbolos[j].tipo;
        strcpy(token.valor, tabelaSimbolos[j].nome);
        token.linha = 0;
        token.coluna = 0;
        salvaTokenEmArquivo(token);
    }
    fclose(arquivoLex);
}

Token proximoToken() {
    Token tokenAtual;
    tokenAtual.linha = linha;
    tokenAtual.coluna = coluna;
    char buffer[100];
    int estado = 0;
    int bufferIndex = 0;
    int stringAberta = 0;
    char c;

    while (codigo[i] != '\0') {
        c = codigo[i++];
        coluna++;

        switch (estado) {
            case 0: // Estado inicial
                if (isspace(c)) {
                    if (c == '\n') {
                        linha++;
                        coluna = 1;
                    }
                    continue;
                } else if (ehLetra(c)) {
                    estado = 1; // Identificador/palavra-chave
                    buffer[bufferIndex++] = c;
                } else if (ehDigito(c)) {
                    estado = 2; // Número
                    buffer[bufferIndex++] = c;
                } else if (strchr("=<>+-*/{};,()", c)) {
                    estado = 3; // Operador/símbolo
                    buffer[bufferIndex++] = c;
                } else if (c == '\'') {
                    estado = 4; // String
                    buffer[bufferIndex++] = c;
                    stringAberta = 1;
                } else {
                    estado = 5; // Caractere desconhecido
                    buffer[bufferIndex++] = c;
                }
                break;

            case 1: // Identificador/palavra-chave
                if (ehLetra(c) || ehDigito(c)) {
                    buffer[bufferIndex++] = c;
                } else {
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    for (int j = 0; j < numPalavrasChave; j++) {
                        if (strcasecmp(buffer, palavrasChave[j]) == 0) {
                            tokenAtual.tipo = PALAVRA_CHAVE;
                            strcpy(tokenAtual.valor, buffer);
                            adicionaNaTabelaSimbolos(buffer, PALAVRA_CHAVE);
                            salvaTokenEmArquivo(tokenAtual);
                            return tokenAtual;
                        }
                    }
                    tokenAtual.tipo = IDENTIFICADOR;
                    strcpy(tokenAtual.valor, buffer);
                    adicionaNaTabelaSimbolos(buffer, IDENTIFICADOR);
                    return tokenAtual;
                }
                break;

            case 2: // Número
                if (ehDigito(c)) {
                    buffer[bufferIndex++] = c;
                } else {
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    tokenAtual.tipo = NUMERO;
                    strcpy(tokenAtual.valor, buffer);
                    return tokenAtual;
                }
                break;

            case 3: // Operador/símbolo
                if ((buffer[0] == ':' && c == '=') || (buffer[0] == '<' && c == '>') ||
                    (buffer[0] == '<' && c == '=') || (buffer[0] == '>' && c == '=')) {
                    buffer[bufferIndex++] = c;
                    buffer[bufferIndex] = '\0';
                    tokenAtual.tipo = identificaOperadorSimbolo(buffer);
                    strcpy(tokenAtual.valor, buffer);
                    return tokenAtual;
                } else {
                    buffer[bufferIndex] = '\0';
                    tokenAtual.tipo = identificaOperadorSimbolo(buffer);
                    strcpy(tokenAtual.valor, buffer);
                    return tokenAtual;
                }
                break;

            case 4: // String
                if (c == '\'') {
                    if (stringAberta) {
                        buffer[bufferIndex] = '\0';
                        tokenAtual.tipo = IDENTIFICADOR;
                        strcpy(tokenAtual.valor, buffer);
                        return tokenAtual;
                    } else {
                        buffer[bufferIndex++] = c;
                        stringAberta = 0;
                    }
                } else {
                    buffer[bufferIndex++] = c;
                }
                break;

            case 5: // Caractere desconhecido
                buffer[bufferIndex] = '\0';
                tokenAtual.tipo = CARACTERE_DESCONHECIDO;
                strcpy(tokenAtual.valor, buffer);
                return tokenAtual;
        }
    }

    // Finalização
    tokenAtual.tipo = CARACTERE_DESCONHECIDO;
    strcpy(tokenAtual.valor, "EOF");
    return tokenAtual;
}

int main() {
    FILE *inputFile;
    char *buffer;
    long length;

    inputFile = fopen("codigo.txt", "rb");
    if (inputFile == NULL) {
        perror("Erro ao abrir o arquivo codigo.txt");
        exit(EXIT_FAILURE);
    }

    fseek(inputFile, 0, SEEK_END);
    length = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, inputFile);
        buffer[length] = '\0';
    }

    fclose(inputFile);

    codigo = buffer;

    gerarRelatorio();

    Token token;
    while ((token = proximoToken()).tipo != CARACTERE_DESCONHECIDO) {
        printf("Tipo: %d, Valor: %s, Linha: %d, Coluna: %d\n", token.tipo, token.valor, token.linha, token.coluna);
    }

    free(buffer);
    return 0;
}
