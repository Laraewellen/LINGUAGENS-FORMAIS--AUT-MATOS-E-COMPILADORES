#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    Identificador, Numero, PalavraChave, OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, OP_ASS, OP_LT, OP_MIN,
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA, StringNaoFechada, CaracterDesconhecido
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
Simbolo tabelaSimbolos[100];
int numSimbolos = 0;

void inicializaTabelaSimbolos() {
    for (int i = 0; i < 10; i++) {
        strcpy(tabelaSimbolos[i].nome, palavrasChave[i]);
        tabelaSimbolos[i].tipo = PalavraChave;
    }
    numSimbolos = 10;
}

int letra(char c) {
    return isalpha(c);
}

int digito(char c) {
    return isdigit(c);
}

TokenType identificaOperadorSimbolo(char *buffer) {
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
    if (strcmp(buffer, "{") == 0 || strcmp(buffer, "}") == 0) return CaracterDesconhecido; // Comentários não permitidos
    if (strcmp(buffer, ",") == 0) return SMB_COM;
    if (strcmp(buffer, ";") == 0) return SMB_SEM;
    if (strcmp(buffer, "(") == 0) return SMB_OPA;
    if (strcmp(buffer, ")") == 0) return SMB_CPA;
    return CaracterDesconhecido;
}

int buscaNaTabelaSimbolos(char *nome) {
    for (int i = 0; i < numSimbolos; i++) {
        if (strcmp(tabelaSimbolos[i].nome, nome) == 0) {
            return 1;
        }
    }
    return 0;
}

void adicionaNaTabelaSimbolos(char *nome, TokenType tipo) {
    if (!buscaNaTabelaSimbolos(nome)) {
        strcpy(tabelaSimbolos[numSimbolos].nome, nome);
        tabelaSimbolos[numSimbolos].tipo = tipo;
        numSimbolos++;
    }
}

void salvaTokenEmArquivo(FILE *arquivoLex, Token token) {
    fprintf(arquivoLex, "<%d, %s> Linha: %d Coluna: %d\n", token.tipo, token.valor, token.linha, token.coluna);
}

void reportaErro(char *mensagem, int linha, int coluna) {
    printf("Erro: %s na linha %d, coluna %d\n", mensagem, linha, coluna);
}

Token proximoToken(FILE *arquivoFonte, FILE *arquivoLex) {
    Token tokenAtual;
    char buffer[100];
    int estado = 0, stringAberta = 0;
    int linha = 1, coluna = 0;
    char c;
    int i = 0;

    while ((c = fgetc(arquivoFonte)) != EOF) {
        coluna++;
        switch (estado) {
            case 0:
                if (isspace(c)) {
                    if (c == '\n') {
                        linha++;
                        coluna = 0;
                    }
                } else if (letra(c)) {
                    estado = 1;
                    buffer[i++] = c;
                } else if (digito(c)) {
                    estado = 2;
                    buffer[i++] = c;
                } else if (strchr("=<>+-*/{},;()", c)) {
                    estado = 3;
                    buffer[i++] = c;
                } else if (c == '\'') {
                    estado = 4;
                    buffer[i++] = c;
                    stringAberta = 1;
                } else {
                    estado = 5;
                    buffer[i++] = c;
                }
                break;
            case 1:
                if (letra(c) || digito(c)) {
                    buffer[i++] = c;
                } else {
                    buffer[i] = '\0';
                    fseek(arquivoFonte, -1, SEEK_CUR);
                    coluna--;
                    if (buscaNaTabelaSimbolos(buffer)) {
                        tokenAtual.tipo = PalavraChave;
                    } else {
                        adicionaNaTabelaSimbolos(buffer, Identificador);
                        tokenAtual.tipo = Identificador;
                    }
                    strcpy(tokenAtual.valor, buffer);
                    tokenAtual.linha = linha;
                    tokenAtual.coluna = coluna - strlen(buffer);
                    salvaTokenEmArquivo(arquivoLex, tokenAtual);
                    return tokenAtual;
                }
                break;
            case 2:
                if (digito(c)) {
                    buffer[i++] = c;
                } else {
                    buffer[i] = '\0';
                    fseek(arquivoFonte, -1, SEEK_CUR);
                    coluna--;
                    tokenAtual.tipo = Numero;
                    strcpy(tokenAtual.valor, buffer);
                    tokenAtual.linha = linha;
                    tokenAtual.coluna = coluna - strlen(buffer);
                    salvaTokenEmArquivo(arquivoLex, tokenAtual);
                    return tokenAtual;
                }
                break;
            case 3:
                if ((buffer[0] == ':' && c == '=') || (buffer[0] == '<' && (c == '>' || c == '=')) || (buffer[0] == '>' && c == '=')) {
                    buffer[i++] = c;
                } else {
                    fseek(arquivoFonte, -1, SEEK_CUR);
                    coluna--;
                }
                buffer[i] = '\0';
                tokenAtual.tipo = identificaOperadorSimbolo(buffer);
                strcpy(tokenAtual.valor, buffer);
                tokenAtual.linha = linha;
                tokenAtual.coluna = coluna - strlen(buffer);
                salvaTokenEmArquivo(arquivoLex, tokenAtual);
                return tokenAtual;
            case 4:
                if (c == '\'') {
                    buffer[i++] = c;
                    buffer[i] = '\0';
                    tokenAtual.tipo = PalavraChave;
                    strcpy(tokenAtual.valor, buffer);
                    tokenAtual.linha = linha;
                    tokenAtual.coluna = coluna - strlen(buffer);
                    salvaTokenEmArquivo(arquivoLex, tokenAtual);
                    return tokenAtual;
                } else if (c == '\n') {
                    buffer[i] = '\0';
                    tokenAtual.tipo = StringNaoFechada;
                    strcpy(tokenAtual.valor, buffer);
                    tokenAtual.linha = linha;
                    tokenAtual.coluna = coluna - strlen(buffer);
                    salvaTokenEmArquivo(arquivoLex, tokenAtual);
                    reportaErro("String não-fechada", linha, coluna - strlen(buffer));
                    return tokenAtual;
                } else {
                    buffer[i++] = c;
                }
                break;
            case 5:
                buffer[i] = '\0';
                tokenAtual.tipo = CaracterDesconhecido;
                strcpy(tokenAtual.valor, buffer);
                tokenAtual.linha = linha;
                tokenAtual.coluna = coluna - strlen(buffer);
                salvaTokenEmArquivo(arquivoLex, tokenAtual);
                reportaErro("Caractere desconhecido", linha, coluna - strlen(buffer));
                return tokenAtual;
        }
    }

    if (stringAberta) {
        buffer[i] = '\0';
        tokenAtual.tipo = StringNaoFechada;
        strcpy(tokenAtual.valor, buffer);
        tokenAtual.linha = linha;
        tokenAtual.coluna = coluna - strlen(buffer);
        salvaTokenEmArquivo(arquivoLex, tokenAtual);
        reportaErro("String não-fechada", linha, coluna - strlen(buffer));
    }

    tokenAtual.tipo = CaracterDesconhecido;
    return tokenAtual;
}

int main() {
    FILE *arquivoFonte = fopen("analisador.txt", "r");
    FILE *arquivoLex = fopen("tokens.lex", "w");

    if (!arquivoFonte) {
        printf("Erro ao abrir o arquivo fonte.\n");
        return 1;
    }

    inicializaTabelaSimbolos();

    while (!feof(arquivoFonte)) {
        proximoToken(arquivoFonte, arquivoLex);
    }

    fclose(arquivoFonte);
    fclose(arquivoLex);

    return 0;
}
