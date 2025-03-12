/*
 * Date: 03/13/2025
 * Name: Brainfuck to C Transpiler
 * Repository https://github.com/BaseMax/brainfuck2c
 *
 * This transpiler converts Brainfuck source code into equivalent C code.
 * It is structured in three phases:
 *
 *  1. Lexer Phase:
 *       Reads the input Brainfuck code and produces an array of tokens.
 *
 *  2. Parser Phase:
 *       Converts the token stream into an Abstract Syntax Tree (AST),
 *       merging consecutive operations and handling loops recursively.
 *       Unmatched brackets are detected and reported.
 *
 *  3. Generator Phase:
 *       Traverses the AST and prints out the corresponding C code.
 *
 * Usage:
 *   Compile: gcc brainfuck2c.c -o brainfuck2c
 *   Run:     ./brainfuck2c [input.bf] > output.c
 *
 * If no input file is specified, it reads from standard input.
 *
 * The generated C code creates a memory tape of TAPE_SIZE cells and uses
 * standard C I/O (getchar/putchar) for Brainfuck’s input/output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAPE_SIZE 30000

/*---------------------------------------------------------------
 * Lexer Phase: Token Definitions and Lexing Function
 *--------------------------------------------------------------*/

// Define the supported Brainfuck token types.
typedef enum {
    TOKEN_PLUS,         // '+'
    TOKEN_MINUS,        // '-'
    TOKEN_NEXT,         // '>'
    TOKEN_PREVIOUS,     // '<'
    TOKEN_OUTPUT,       // '.'
    TOKEN_INPUT,        // ','
    TOKEN_LOOP_START,   // '['
    TOKEN_LOOP_END      // ']'
} TokenType;

typedef struct {
    TokenType type;
    int pos;
} Token;

/*
 * lex()
 *
 * Converts the input Brainfuck source string into a dynamic array of tokens.
 * Non-Brainfuck characters are ignored.
 *
 * The number of tokens is returned in *numTokens.
 * The caller must free the returned array.
 */
Token* lex(const char* src, int *numTokens) {
    int capacity = 128;
    int count = 0;
    Token* tokens = malloc(capacity * sizeof(Token));
    if (!tokens) {
        perror("Memory allocation failed in lex()");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; src[i] != '\0'; i++) {
        char c = src[i];
        TokenType t;
        switch(c) {
            case '+': t = TOKEN_PLUS; break;
            case '-': t = TOKEN_MINUS; break;
            case '>': t = TOKEN_NEXT; break;
            case '<': t = TOKEN_PREVIOUS; break;
            case '.': t = TOKEN_OUTPUT; break;
            case ',': t = TOKEN_INPUT; break;
            case '[': t = TOKEN_LOOP_START; break;
            case ']': t = TOKEN_LOOP_END; break;
            default: continue;
        }
        if (count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(Token));
            if (!tokens) {
                perror("Memory reallocation failed in lex()");
                exit(EXIT_FAILURE);
            }
        }
        tokens[count].type = t;
        tokens[count].pos = i;
        count++;
    }
    *numTokens = count;
    return tokens;
}

/*---------------------------------------------------------------
 * Parser Phase: AST Definitions and Parsing Functions
 *--------------------------------------------------------------*/
typedef struct ASTNode {
    TokenType type;
    int count;
    struct ASTNode *children;
    int numChildren;
} ASTNode;

/*
 * parseLevel()
 *
 * Recursively parses tokens into an AST for one "level" (the top level or inside a loop).
 *
 * Parameters:
 *   tokens     - The complete token array.
 *   numTokens  - Total number of tokens.
 *   index      - Pointer to the current index in the token array.
 *   countOut   - Returns the number of AST nodes created at this level.
 *   insideLoop - If nonzero, the function expects to stop at a TOKEN_LOOP_END.
 *
 * On error (e.g. unmatched brackets), the function prints an error and exits.
 */
ASTNode* parseLevel(Token* tokens, int numTokens, int *index, int *countOut, int insideLoop) {
    int capacity = 10;
    int count = 0;
    ASTNode *nodes = malloc(capacity * sizeof(ASTNode));
    if (!nodes) {
        perror("Memory allocation failed in parseLevel()");
        exit(EXIT_FAILURE);
    }
    
    while (*index < numTokens) {
        Token current = tokens[*index];
        if (current.type == TOKEN_LOOP_END) {
            if (!insideLoop) {
                fprintf(stderr, "Error: Unmatched ']' at position %d\n", current.pos);
                exit(EXIT_FAILURE);
            }
            (*index)++; // TOKEN_LOOP_END
            *countOut = count;
            return nodes;
        }
        
        if (current.type == TOKEN_LOOP_START) {
            (*index)++; // TOKEN_LOOP_START
            int childCount = 0;
            ASTNode *children = parseLevel(tokens, numTokens, index, &childCount, 1);
            
            ASTNode node;
            node.type = TOKEN_LOOP_START;
            node.count = 0;
            node.children = children;
            node.numChildren = childCount;
            
            if (count >= capacity) {
                capacity *= 2;
                nodes = realloc(nodes, capacity * sizeof(ASTNode));
                if (!nodes) {
                    perror("Memory reallocation failed in parseLevel()");
                    exit(EXIT_FAILURE);
                }
            }
            nodes[count++] = node;
        }
        else if (current.type == TOKEN_PLUS || current.type == TOKEN_MINUS ||
                 current.type == TOKEN_NEXT || current.type == TOKEN_PREVIOUS ||
                 current.type == TOKEN_OUTPUT || current.type == TOKEN_INPUT) {
            // Merge consecutive tokens of the same type.
            TokenType type = current.type;
            int repeat = 0;
            while (*index < numTokens && tokens[*index].type == type) {
                repeat++;
                (*index)++;
            }
            ASTNode node;
            node.type = type;
            node.count = repeat;
            node.children = NULL;
            node.numChildren = 0;
            
            if (count >= capacity) {
                capacity *= 2;
                nodes = realloc(nodes, capacity * sizeof(ASTNode));
                if (!nodes) {
                    perror("Memory reallocation failed in parseLevel()");
                    exit(EXIT_FAILURE);
                }
            }
            nodes[count++] = node;
        }
        else {
            (*index)++;
        }
    }
    
    if (insideLoop) {
        fprintf(stderr, "Error: Unmatched '[' detected\n");
        exit(EXIT_FAILURE);
    }
    
    *countOut = count;
    return nodes;
}

/*
 * parseTokens()
 *
 * Entry point for parsing the entire token stream into an AST.
 * Returns a pointer to an array of AST nodes representing the root level.
 * The number of nodes is returned in *countOut.
 */
ASTNode* parseTokens(Token* tokens, int numTokens, int *countOut) {
    int index = 0;
    return parseLevel(tokens, numTokens, &index, countOut, 0);
}

/*---------------------------------------------------------------
 * Generator Phase: Code Generation Functions
 *--------------------------------------------------------------*/
void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("    ");
    }
}

/*
 * generate_code()
 *
 * Recursively traverses the AST and prints out equivalent C code.
 *
 * Parameters:
 *   nodes        - Pointer to the current AST node array.
 *   numNodes     - Number of nodes at this level.
 *   indent_level - Current indentation level for pretty printing.
 */
void generate_code(ASTNode* nodes, int numNodes, int indent_level) {
    for (int i = 0; i < numNodes; i++) {
        ASTNode node = nodes[i];
        switch (node.type) {
            case TOKEN_PLUS:
                print_indent(indent_level);
                printf("*ptr += %d;\n", node.count);
                break;
            case TOKEN_MINUS:
                print_indent(indent_level);
                printf("*ptr -= %d;\n", node.count);
                break;
            case TOKEN_NEXT:
                print_indent(indent_level);
                printf("ptr += %d;\n", node.count);
                break;
            case TOKEN_PREVIOUS:
                print_indent(indent_level);
                printf("ptr -= %d;\n", node.count);
                break;
            case TOKEN_OUTPUT:
                if (node.count == 1) {
                    print_indent(indent_level);
                    printf("putchar(*ptr);\n");
                } else {
                    print_indent(indent_level);
                    printf("for (int i = 0; i < %d; i++) {\n", node.count);
                    print_indent(indent_level + 1);
                    printf("putchar(*ptr);\n");
                    print_indent(indent_level);
                    printf("}\n");
                }
                break;
            case TOKEN_INPUT:
                if (node.count == 1) {
                    print_indent(indent_level);
                    printf("*ptr = getchar();\n");
                } else {
                    print_indent(indent_level);
                    printf("for (int i = 0; i < %d; i++) {\n", node.count);
                    print_indent(indent_level + 1);
                    printf("*ptr = getchar();\n");
                    print_indent(indent_level);
                    printf("}\n");
                }
                break;
            case TOKEN_LOOP_START:
                print_indent(indent_level);
                printf("while (*ptr) {\n");
                generate_code(node.children, node.numChildren, indent_level + 1);
                print_indent(indent_level);
                printf("}\n");
                break;
            default:
                break;
        }
    }
}

/*
 * free_ast()
 *
 * Recursively frees memory allocated for the AST.
 */
void free_ast(ASTNode* nodes, int numNodes) {
    for (int i = 0; i < numNodes; i++) {
        if (nodes[i].type == TOKEN_LOOP_START && nodes[i].children != NULL) {
            free_ast(nodes[i].children, nodes[i].numChildren);
            free(nodes[i].children);
        }
    }
}

/*---------------------------------------------------------------
 * Main Function: Integrating Lexer, Parser, and Generator
 *--------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    FILE *fp = stdin;
    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
    }
    
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *source = malloc(fsize + 1);
    if (!source) {
        perror("Memory allocation failed for source code");
        exit(EXIT_FAILURE);
    }
    fread(source, 1, fsize, fp);
    source[fsize] = '\0';
    if (fp != stdin) {
        fclose(fp);
    }
    
    // --- Lexer Phase ---
    int numTokens = 0;
    Token* tokens = lex(source, &numTokens);
    free(source);
    
    // --- Parser Phase ---
    int numASTNodes = 0;
    ASTNode* ast = parseTokens(tokens, numTokens, &numASTNodes);
    free(tokens);
    
    // --- Generator Phase ---
    printf("#include <stdio.h>\n");
    printf("#include <stdlib.h>\n\n");
    printf("#define TAPE_SIZE %d\n\n", TAPE_SIZE);
    printf("int main(void) {\n");
    printf("    unsigned char array[TAPE_SIZE] = {0};\n");
    printf("    unsigned char *ptr = array;\n\n");
    
    generate_code(ast, numASTNodes, 1);
    
    printf("\n    return 0;\n");
    printf("}\n");
    
    free_ast(ast, numASTNodes);
    free(ast);
    
    return 0;
}
