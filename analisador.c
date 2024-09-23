#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TABELA 100
#define MAX_TOKEN_LENGTH 100

typedef enum {
    Identificador, Numero, PalavraChave, OP_EQ, OP_GE, OP_MUL, OP_NE, OP_LE, OP_DIV, OP_GT, OP_AD, OP_ASS, OP_LT, OP_MIN,
    SMB_OBC, SMB_COM, SMB_CBC, SMB_SEM, SMB_OPA, SMB_CPA, StringNaoFechada, CaracterDesconhecido, EOF_TOKEN
} TokenType;

typedef struct {
    char nome[MAX_TOKEN_LENGTH];
    char lexema[MAX_TOKEN_LENGTH];
    int linha, coluna;
} Token;

typedef struct {
    char lexema[MAX_TOKEN_LENGTH];
    TokenType tipo;
} Simbolo;

Simbolo tabelaSimbolos[MAX_TABELA];
int tamanhoTabelaSimbolos = 0;

const char *palavrasChave[] = {"if", "then", "else", "begin", "end", "program", "var", "integer", "real", "while"};
int numPalavrasChave = 10;

int linha_atual = 1, coluna_atual = 0;

void reportar_erro(const char *mensagem, int linha, int coluna) {
    printf("\033[31mErro léxico: %s na linha %d, coluna %d\033[0m\n", mensagem, linha, coluna);
}

int letra(char c) {
    return isalpha(c);
}

int digito(char c) {
    return isdigit(c);
}

int palavraChave(char *buffer) {
    for (int i = 0; i < numPalavrasChave; i++) {
        if (strcmp(palavrasChave[i], buffer) == 0) return 1;
    }
    return 0;
}

const char* identificaOperador(char *buffer) {
    if (strcmp(buffer, "=") == 0) return "OP_EQ";
    if (strcmp(buffer, ">=") == 0) return "OP_GE";
    if (strcmp(buffer, "*") == 0) return "OP_MUL";
    if (strcmp(buffer, "<>") == 0) return "OP_NE";
    if (strcmp(buffer, "<=") == 0) return "OP_LE";
    if (strcmp(buffer, "/") == 0) return "OP_DIV";
    if (strcmp(buffer, ">") == 0) return "OP_GT";
    if (strcmp(buffer, "+") == 0) return "OP_AD";
    if (strcmp(buffer, ":=") == 0) return "OP_ASS";
    if (strcmp(buffer, "<") == 0) return "OP_LT";
    if (strcmp(buffer, "-") == 0) return "OP_MIN";
    return "CaracterDesconhecido";
}

// Função para consultar se um símbolo já está na tabela de símbolos
int consultaTabelaSimbolos(char *lexema) {
    for (int i = 0; i < tamanhoTabelaSimbolos; i++) {
        if (strcmp(tabelaSimbolos[i].lexema, lexema) == 0) {
            return i;  // Retorna o índice se o símbolo já estiver na tabela
        }
    }
    return -1;  // Retorna -1 se o símbolo não estiver na tabela
}

// Função para adicionar um símbolo à tabela de símbolos
void adicionaTabelaSimbolos(char *lexema, TokenType tipo) {
    if (consultaTabelaSimbolos(lexema) == -1 && (tipo == Identificador || tipo == PalavraChave)) {
        strcpy(tabelaSimbolos[tamanhoTabelaSimbolos].lexema, lexema);
        tabelaSimbolos[tamanhoTabelaSimbolos].tipo = tipo;
        tamanhoTabelaSimbolos++;
    }
}

Token obter_token(FILE *arquivoFonte) {
    char c, buffer[MAX_TOKEN_LENGTH];
    int i = 0;
    Token token;

    while ((c = fgetc(arquivoFonte)) != EOF) {
        coluna_atual++;

        if (isspace(c)) {
            if (c == '\n') {
                linha_atual++;
                coluna_atual = 0;
            }
            continue;
        } else if (letra(c)) {
            buffer[i++] = c;
            while ((c = fgetc(arquivoFonte)) != EOF && (letra(c) || digito(c))) {
                buffer[i++] = c;
                coluna_atual++;
            }
            buffer[i] = '\0';
            ungetc(c, arquivoFonte);

            if (palavraChave(buffer)) {
                strcpy(token.nome, "WORD");
                adicionaTabelaSimbolos(buffer, PalavraChave);
            } else {
                strcpy(token.nome, "ID");
                adicionaTabelaSimbolos(buffer, Identificador);
            }
            strcpy(token.lexema, buffer);
            token.linha = linha_atual;
            token.coluna = coluna_atual - i + 1;
            return token;
        } else if (digito(c)) {
            buffer[i++] = c;
            while ((c = fgetc(arquivoFonte)) != EOF && digito(c)) {
                buffer[i++] = c;
                coluna_atual++;
            }
            buffer[i] = '\0';
            ungetc(c, arquivoFonte);

            strcpy(token.nome, "NUM");
            strcpy(token.lexema, buffer);
            token.linha = linha_atual;
            token.coluna = coluna_atual - i + 1;
            return token;
        } else if (strchr("=<>+-*/{},;()", c)) {
            buffer[i++] = c;
            if (c == ':' || c == '<' || c == '>') {
                char prox = fgetc(arquivoFonte);
                if ((c == ':' && prox == '=') || (c == '<' && (prox == '>' || prox == '=')) || (c == '>' && prox == '=')) {
                    buffer[i++] = prox;
                    coluna_atual++;
                } else {
                    ungetc(prox, arquivoFonte);
                }
            }
            buffer[i] = '\0';
            strcpy(token.nome, identificaOperador(buffer)); // Ajuste aqui para usar a função que retorna o nome do operador
            strcpy(token.lexema, buffer);
            token.linha = linha_atual;
            token.coluna = coluna_atual - i + 1;
            return token;
        } else {
            reportar_erro("Caractere não reconhecido", linha_atual, coluna_atual);
        }
    }

    strcpy(token.nome, "EOF");
    strcpy(token.lexema, "EOF");
    token.linha = linha_atual;
    token.coluna = coluna_atual;
    return token;
}

void analisar_lexico(FILE *arquivoFonte, FILE *arquivoLex) {
    Token token;
    do {
        token = obter_token(arquivoFonte);
        fprintf(arquivoLex, "<%s, %s> Linha: %d, Coluna: %d\n", token.nome, token.lexema, token.linha, token.coluna);
    } while (strcmp(token.nome, "EOF") != 0);
}

int main() {
    FILE *arquivoFonte = fopen("analisador.txt", "r");
    FILE *arquivoLex = fopen("tokens.lex", "w");

    if (!arquivoFonte) {
        printf("Erro ao abrir o arquivo fonte.\n");
        return 1;
    }

    analisar_lexico(arquivoFonte, arquivoLex);

    fclose(arquivoFonte);
    fclose(arquivoLex);
    return 0;
}
