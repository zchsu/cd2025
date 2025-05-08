#include <stdio.h>
#include <string.h>

#define MAX_TOKEN_LEN 100
#define MAX_TREE_NODES 1000

// Structure for tokens from scanner
typedef struct TokenNode {
    char lexeme[MAX_TOKEN_LEN]; 
    char type[30];               
    struct TokenNode *next;
} TokenNode;

// Structure for parse tree nodes
typedef struct TreeNode {
    char name[30];           // Non-terminal name (S, S', E)
    char rule[50];           // Production rule applied (E S', epsilon, +S, num, (S))
    char value[MAX_TOKEN_LEN]; // For terminals (actual token value)
    struct TreeNode *children[10]; // Max 10 children per node
    int numChildren;
} TreeNode;

// Global variables for memory management
TokenNode tokenPool[100];
int tokenCount = 0;
TreeNode treeNodePool[MAX_TREE_NODES];
int treeNodeCount = 0;

TokenNode *tokenHead = NULL;
TokenNode *tokenTail = NULL;

// Current token being processed
char currentToken[30];
char currentLexeme[MAX_TOKEN_LEN];

// Scanner functions from HW#1
void addToken(const char *lexeme, const char *type) {
    if (tokenCount >= 100) {
        printf("Error: Token pool overflow\n");
        return;
    }
    
    TokenNode *newNode = &tokenPool[tokenCount++];
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
    tokenHead = tokenTail = NULL;
    tokenCount = 0;
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

// Parser error handling
void parseError(const char* message) {
    printf("Parse Error: %s\n", message);
    printf("Current token: %s, lexeme: %s\n", currentToken, currentLexeme);
    return;
}

// Get the next token from the scanner
int getNextToken() {
    static TokenNode *current = NULL;
    
    if (current == NULL) {
        current = tokenHead;
    } else {
        current = current->next;
    }
    
    if (current == NULL) {
        strcpy(currentToken, "EOF");
        strcpy(currentLexeme, "");
        return 0;
    }
    
    strcpy(currentToken, current->type);
    strcpy(currentLexeme, current->lexeme);
    return 1;
}

// Create a new tree node
TreeNode* createNode(const char* name, const char* rule) {
    if (treeNodeCount >= MAX_TREE_NODES) {
        printf("Error: Tree node pool overflow\n");
        return NULL;
    }
    
    TreeNode* node = &treeNodePool[treeNodeCount++];
    strcpy(node->name, name);
    strcpy(node->rule, rule);
    node->value[0] = '\0';
    node->numChildren = 0;
    return node;
}

// Add a child to a tree node
void addChild(TreeNode* parent, TreeNode* child) {
    if (parent == NULL || child == NULL) return;
    
    if (parent->numChildren < 10) {
        parent->children[parent->numChildren++] = child;
    }
}

// Set the value of a tree node (for terminals)
void setValue(TreeNode* node, const char* value) {
    if (node == NULL) return;
    strcpy(node->value, value);
}

// Print the parse tree with the correct format
void printParseTree(TreeNode* node, int depth) {
    if (node == NULL) return;
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("     ");
    }
    
    // Print node with its rule
    if (strlen(node->rule) > 0) {
        printf("%s -> %s\n", node->name, node->rule);
    } 
    // For terminal nodes with values
    else if (strlen(node->value) > 0) {
        printf("%s\n", node->value);
    }
    
    // Print children
    for (int i = 0; i < node->numChildren; i++) {
        printParseTree(node->children[i], depth + 1);
    }
}

// Forward declarations for the parsing functions
TreeNode* parse_S();
TreeNode* parse_S_prime();
TreeNode* parse_E();

// Parse S → E S'
TreeNode* parse_S() {
    TreeNode* node = createNode("S", "E S'");
    printf("Parsing S -> E S'\n");
    
    TreeNode* eNode = parse_E();
    TreeNode* sPrimeNode = parse_S_prime();
    
    addChild(node, eNode);
    addChild(node, sPrimeNode);
    
    return node;
}

// Parse S' → ε | +S
TreeNode* parse_S_prime() {
    TreeNode* node;
    printf("Parsing S'\n");
    
    if (strcmp(currentToken, "PLUS_TOKEN") == 0) {
        node = createNode("S'", "+S");
        printf("Matched '+'\n");
        
        // Create a terminal node for the '+' symbol
        TreeNode* plusNode = createNode("+", "");
        setValue(plusNode, currentLexeme);
        addChild(node, plusNode);
        
        getNextToken();
        TreeNode* sNode = parse_S();
        addChild(node, sNode);
    } else {
        // Epsilon production - use "epsilon" instead of Unicode "ε"
        node = createNode("S'", "epsilon");
        printf("Using epsilon production for S'\n");
    }
    
    return node;
}

// Parse E → num | (S)
TreeNode* parse_E() {
    TreeNode* node;
    printf("Parsing E\n");
    
    if (strcmp(currentToken, "LITERAL_TOKEN") == 0) {
        node = createNode("E", "num");
        printf("Matched number: %s\n", currentLexeme);
        
        // Create a terminal node for the number
        TreeNode* numNode = createNode("num", "");
        setValue(numNode, currentLexeme);
        addChild(node, numNode);
        
        getNextToken();
    } else if (strcmp(currentToken, "LEFTPAREN_TOKEN") == 0) {
        node = createNode("E", "(S)");
        printf("Matched '('\n");
        
        // Create a terminal node for (
        TreeNode* leftParenNode = createNode("(", "");
        setValue(leftParenNode, "(");
        addChild(node, leftParenNode);
        
        getNextToken();
        
        TreeNode* sNode = parse_S();
        addChild(node, sNode);
        
        if (strcmp(currentToken, "RIGHTPAREN_TOKEN") == 0) {
            printf("Matched ')'\n");
            
            // Create a terminal node for )
            TreeNode* rightParenNode = createNode(")", "");
            setValue(rightParenNode, ")");
            addChild(node, rightParenNode);
            
            getNextToken();
        } else {
            parseError("Expected ')'");
        }
    } else {
        parseError("Expected number or '('");
    }
    
    return node;
}

// Create a test file with the given content
FILE* createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
    }
    return fopen(filename, "r");
}

int main() {
    FILE* file = createTestFile("test.c", "(1+2+(3+4))+5");
    if (!file) {
        printf("Failed to create test file\n");
        return 1;
    }

    // Scan the input file to generate tokens
    scan(file);
    fclose(file);
    
    printf("Tokens from Scanner:\n");
    printTokens();
    
    printf("\nStarting Parser:\n");
    // Get the first token
    getNextToken();
    
    // Start parsing from the start symbol S
    TreeNode* parseTree = parse_S();
    
    // Check if we've processed all tokens
    if (strcmp(currentToken, "EOF") != 0) {
        parseError("Unexpected tokens after parsing completed");
    }
    
    printf("\nParsing completed successfully!\n");
    printf("\nParse Tree:\n");
    printParseTree(parseTree, 0);
    
    // Free memory
    freeTokens();
    
    return 0;
}