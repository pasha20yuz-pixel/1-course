#include "memstat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ---------- AST node ----------
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP      // унарный минус/плюс
} NodeType;

typedef struct AstNode {
    NodeType type;
    union {
        long long value;
        char var_name[64];
        struct {
            char op;
            struct AstNode* left;
            struct AstNode* right;
        } binary;
        struct {
            char op;               // '+' или '-'
            struct AstNode* operand;
        } unary;
    };
} AstNode;

// ---------- create / free ----------
AstNode* create_number_node(long long val) {
    AstNode* node = (AstNode*)MALLOC(sizeof(AstNode));
    node->type = NODE_NUMBER;
    node->value = val;
    return node;
}

AstNode* create_variable_node(const char* name) {
    AstNode* node = (AstNode*)MALLOC(sizeof(AstNode));
    node->type = NODE_VARIABLE;
    strncpy(node->var_name, name, 63);
    node->var_name[63] = '\0';
    return node;
}

AstNode* create_binary_node(char op, AstNode* left, AstNode* right) {
    AstNode* node = (AstNode*)MALLOC(sizeof(AstNode));
    node->type = NODE_BINARY_OP;
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;
    return node;
}

AstNode* create_unary_node(char op, AstNode* operand) {
    AstNode* node = (AstNode*)MALLOC(sizeof(AstNode));
    node->type = NODE_UNARY_OP;
    node->unary.op = op;
    node->unary.operand = operand;
    return node;
}

void free_ast(AstNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_BINARY_OP:
            free_ast(node->binary.left);
            free_ast(node->binary.right);
            break;
        case NODE_UNARY_OP:
            free_ast(node->unary.operand);
            break;
        default:
            break;
    }
    FREE(node);
}

// ---------- обходы (префикс, постфикс) ----------
static void prefix_rec(AstNode* node, char* buf, size_t* pos, size_t size) {
    if (!node) return;
    int n;
    switch (node->type) {
        case NODE_NUMBER:
            n = snprintf(buf + *pos, size - *pos, "%lld", node->value);
            *pos += n;
            break;
        case NODE_VARIABLE:
            n = snprintf(buf + *pos, size - *pos, "%s", node->var_name);
            *pos += n;
            break;
        case NODE_BINARY_OP:
            n = snprintf(buf + *pos, size - *pos, "%c", node->binary.op);
            *pos += n;
            if (*pos < size) buf[(*pos)++] = ' ';
            prefix_rec(node->binary.left, buf, pos, size);
            if (*pos < size) buf[(*pos)++] = ' ';
            prefix_rec(node->binary.right, buf, pos, size);
            break;
        case NODE_UNARY_OP:
            n = snprintf(buf + *pos, size - *pos, "%c", node->unary.op);
            *pos += n;
            if (*pos < size) buf[(*pos)++] = ' ';
            prefix_rec(node->unary.operand, buf, pos, size);
            break;
    }
}

void to_prefix(AstNode* node, char* buffer, size_t size) {
    if (!node) { buffer[0] = '\0'; return; }
    size_t pos = 0;
    prefix_rec(node, buffer, &pos, size);
    if (pos < size) buffer[pos] = '\0';
    else buffer[size-1] = '\0';
}

static void postfix_rec(AstNode* node, char* buf, size_t* pos, size_t size) {
    if (!node) return;
    int n;
    switch (node->type) {
        case NODE_NUMBER:
            n = snprintf(buf + *pos, size - *pos, "%lld", node->value);
            *pos += n;
            break;
        case NODE_VARIABLE:
            n = snprintf(buf + *pos, size - *pos, "%s", node->var_name);
            *pos += n;
            break;
        case NODE_BINARY_OP:
            postfix_rec(node->binary.left, buf, pos, size);
            if (*pos < size) buf[(*pos)++] = ' ';
            postfix_rec(node->binary.right, buf, pos, size);
            if (*pos < size) buf[(*pos)++] = ' ';
            n = snprintf(buf + *pos, size - *pos, "%c", node->binary.op);
            *pos += n;
            break;
        case NODE_UNARY_OP:
            postfix_rec(node->unary.operand, buf, pos, size);
            if (*pos < size) buf[(*pos)++] = ' ';
            n = snprintf(buf + *pos, size - *pos, "%c", node->unary.op);
            *pos += n;
            break;
    }
}

void to_postfix(AstNode* node, char* buffer, size_t size) {
    if (!node) { buffer[0] = '\0'; return; }
    size_t pos = 0;
    postfix_rec(node, buffer, &pos, size);
    if (pos < size) buffer[pos] = '\0';
    else buffer[size-1] = '\0';
}

// ---------- вычисление ----------
typedef struct {
    char name[64];
    long long value;
} Variable;

static long long gcd(long long a, long long b) {
    a = llabs(a); b = llabs(b);
    while (b) { long long t = b; b = a % b; a = t; }
    return a;
}

static long long pow_ll(long long base, long long exp) {
    if (exp < 0) return 0;
    long long res = 1;
    for (long long i = 0; i < exp; i++) res *= base;
    return res;
}

long long evaluate(AstNode* node, Variable* vars, int var_count, int* error) {
    if (!node) { *error = 1; return 0; }
    switch (node->type) {
        case NODE_NUMBER:
            return node->value;
        case NODE_VARIABLE:
            for (int i = 0; i < var_count; i++)
                if (strcmp(vars[i].name, node->var_name) == 0)
                    return vars[i].value;
            *error = 1;
            return 0;
        case NODE_BINARY_OP: {
            long long left = evaluate(node->binary.left, vars, var_count, error);
            if (*error) return 0;
            long long right = evaluate(node->binary.right, vars, var_count, error);
            if (*error) return 0;
            char op = node->binary.op;
            switch (op) {
                case '+': return left + right;
                case '-': return left - right;
                case '*': return left * right;
                case '/':
                    if (right == 0) { *error = 1; return 0; }
                    return left / right;
                case '%':
                    if (right == 0) { *error = 1; return 0; }
                    return left % right;
                case '^':
                    if (right < 0) { *error = 1; return 0; }
                    return pow_ll(left, right);
                case '#':
                    return gcd(left, right);
                default:
                    *error = 1;
                    return 0;
            }
        }
        case NODE_UNARY_OP: {
            long long val = evaluate(node->unary.operand, vars, var_count, error);
            if (*error) return 0;
            if (node->unary.op == '-') return -val;
            else return val;   // унарный плюс
        }
        default:
            *error = 1;
            return 0;
    }
}
