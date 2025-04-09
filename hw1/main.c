#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 100

typedef struct TokenNode {
    char lexeme[MAX_TOKEN_LEN]; 
    char type[30];               
    struct TokenNode *next;
} TokenNode;

TokenNode *tokenHead = NULL;
TokenNode *tokenTail = NULL;

void addToken(const char *lexeme, const char *type) {
    TokenNode *newNode = (TokenNode *)malloc(sizeof(TokenNode));
    strcpy(newNode->lexeme, lexeme);
    strcpy(newNode->type, type);
    newNode->next = NULL;

    if (!tokenHead) {
        tokenHead = tokenTail = newNode;
    } else {
        tokenTail->next = newNode;
        tokenTail = newNode;
    }
}

void printTokens() {
    TokenNode *current = tokenHead;
    while (current) {
        printf("%s: %s\n", current->lexeme, current->type);
        current = current->next;
    }
}

void freeTokens() {
    TokenNode *current = tokenHead;
    while (current) {
        TokenNode *temp = current;
        current = current->next;
        free(temp);
    }
    tokenHead = tokenTail = NULL;
}

int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_letter_or_digit(char c) {
    return is_letter(c) || is_digit(c);
}

const char *getKeywordType(const char *str) {
    if (strcmp(str, "int") == 0) return "TYPE_TOKEN";
    if (strcmp(str, "main") == 0) return "MAIN_TOKEN";
    if (strcmp(str, "if") == 0) return "IF_TOKEN";
    if (strcmp(str, "else") == 0) return "ELSE_TOKEN";
    if (strcmp(str, "while") == 0) return "WHILE_TOKEN";
    return NULL;
}

void scan(FILE *file) {
    char ch;
    char buffer[MAX_TOKEN_LEN];
    int idx;

    while ((ch = fgetc(file)) != EOF) {
   
        if (ch == ' ' || ch == '\t' || ch == '\n') continue;

        if (is_letter(ch) || ch == '_') {
            idx = 0;
            buffer[idx++] = ch;
            while ((ch = fgetc(file)) != EOF && (is_letter_or_digit(ch) || ch == '_')) {
                buffer[idx++] = ch;
            }
            buffer[idx] = '\0';
            if (ch != EOF) ungetc(ch, file);

            const char *type = getKeywordType(buffer);
            if (type)
                addToken(buffer, type);
            else
                addToken(buffer, "ID_TOKEN");
        }

        else if (is_digit(ch)) {
            idx = 0;
            buffer[idx++] = ch;
            while ((ch = fgetc(file)) != EOF && is_digit(ch)) {
                buffer[idx++] = ch;
            }
            buffer[idx] = '\0';
            if (ch != EOF) ungetc(ch, file);
            addToken(buffer, "LITERAL_TOKEN");
        }

        else {
            switch (ch) {
                case '=':
                    if ((ch = fgetc(file)) == '=') addToken("==", "EQUAL_TOKEN");
                    else { ungetc(ch, file); addToken("=", "ASSIGN_TOKEN"); }
                    break;
                case '>':
                    if ((ch = fgetc(file)) == '=') addToken(">=", "GREATEREQUAL_TOKEN");
                    else { ungetc(ch, file); addToken(">", "GREATER_TOKEN"); }
                    break;
                case '<':
                    if ((ch = fgetc(file)) == '=') addToken("<=", "LESSEQUAL_TOKEN");
                    else { ungetc(ch, file); addToken("<", "LESS_TOKEN"); }
                    break;
                case '+': addToken("+", "PLUS_TOKEN"); break;
                case '-': addToken("-", "MINUS_TOKEN"); break;
                case '(': addToken("(", "LEFTPAREN_TOKEN"); break;
                case ')': addToken(")", "RIGHTPAREN_TOKEN"); break;
                case '{': addToken("{", "LEFTBRACE_TOKEN"); break;
                case '}': addToken("}", "RIGHTBRACE_TOKEN"); break;
                case ';': addToken(";", "SEMICOLON_TOKEN"); break;
                default: break; 
            }
        }
    }
}

int main() {
    FILE *file = fopen("test.c", "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    scan(file);
    fclose(file);

    printTokens();
    freeTokens();
    return 0;
}
