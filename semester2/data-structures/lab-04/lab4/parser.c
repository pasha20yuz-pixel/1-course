#include "memstat.h"
#include "ast.c"
#include <ctype.h>
#include <string.h>

typedef enum {
    TOKEN_NUMBER, TOKEN_VARIABLE, TOKEN_OPERATOR,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_END, TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
    long long value;
    int is_unary;
} Token;

static const char* stream;
static size_t pos;

void lexer_init(const char* input) { stream = input; pos = 0; }
static void skip_spaces() { while (stream[pos] && isspace(stream[pos])) pos++; }
static int is_operator_char(char c) { return strchr("+-*/%^#", c) != NULL; }

Token lexer_next() {
    Token tok = {0};
    skip_spaces();
    if (stream[pos] == '\0') { tok.type = TOKEN_END; return tok; }
    char c = stream[pos];
    if (isdigit(c)) {
        tok.type = TOKEN_NUMBER;
        int i = 0;
        while (isdigit(stream[pos]) && i < 63) tok.text[i++] = stream[pos++];
        tok.text[i] = '\0';
        tok.value = atoll(tok.text);
        return tok;
    }
    if (isalpha(c)) {
        tok.type = TOKEN_VARIABLE;
        int i = 0;
        while (isalnum(stream[pos]) && i < 63) tok.text[i++] = stream[pos++];
        tok.text[i] = '\0';
        return tok;
    }
    if (c == '(') { tok.type = TOKEN_LPAREN; pos++; return tok; }
    if (c == ')') { tok.type = TOKEN_RPAREN; pos++; return tok; }
    if (is_operator_char(c)) { tok.type = TOKEN_OPERATOR; tok.text[0] = c; pos++; return tok; }
    tok.type = TOKEN_ERROR;
    return tok;
}

typedef struct { AstNode** arr; int top; int cap; } NodeStack;
static NodeStack* create_stack(int cap) {
    NodeStack* s = (NodeStack*)MALLOC(sizeof(NodeStack));
    s->arr = (AstNode**)MALLOC(cap * sizeof(AstNode*));
    s->top = -1; s->cap = cap;
    return s;
}
static void push_node(NodeStack* s, AstNode* node) { s->arr[++s->top] = node; }
static AstNode* pop_node(NodeStack* s) { return (s->top >= 0) ? s->arr[s->top--] : NULL; }
static AstNode* top_node(NodeStack* s) { return (s->top >= 0) ? s->arr[s->top] : NULL; }
static void free_stack(NodeStack* s) { FREE(s->arr); FREE(s); }

typedef struct { char op; int prec; int right_assoc; } Operator;
static Operator ops[] = {{'#',5,0},{'^',4,1},{'*',3,0},{'/',3,0},{'%',3,0},{'+',2,0},{'-',2,0}};
static int op_count = 7;
static int get_precedence(char op) {
    for (int i = 0; i < op_count; i++) if (ops[i].op == op) return ops[i].prec;
    return 0;
}
static int is_right_assoc(char op) {
    for (int i = 0; i < op_count; i++) if (ops[i].op == op) return ops[i].right_assoc;
    return 0;
}
static int is_binary_operator(char op) { return strchr("+-*/%^#", op) != NULL; }

AstNode* parse_infix(const char* expr, int* error) {
    lexer_init(expr);
    NodeStack* operands = create_stack(256);
    NodeStack* operators = create_stack(256);
    *error = 0;
    Token prev_tok = { .type = TOKEN_END };
    Token tok;
    while ((tok = lexer_next()).type != TOKEN_END && !*error) {
        if (tok.type == TOKEN_NUMBER || tok.type == TOKEN_VARIABLE) {
            if (tok.type == TOKEN_NUMBER) push_node(operands, create_number_node(tok.value));
            else push_node(operands, create_variable_node(tok.text));
            prev_tok = tok;
        }
        else if (tok.type == TOKEN_LPAREN) {
            push_node(operators, (AstNode*)(intptr_t)'(');
            prev_tok = tok;
        }
        else if (tok.type == TOKEN_RPAREN) {
            while (operators->top >= 0 && (char)(intptr_t)top_node(operators) != '(') {
                intptr_t op_val = (intptr_t)pop_node(operators);
                if (op_val < 0) {
                    char uop = -op_val;
                    AstNode* operand = pop_node(operands);
                    if (!operand) { *error = 1; break; }
                    push_node(operands, create_unary_node(uop, operand));
                } else {
                    char bop = (char)op_val;
                    AstNode* right = pop_node(operands);
                    AstNode* left = pop_node(operands);
                    if (!left || !right) { *error = 1; break; }
                    push_node(operands, create_binary_node(bop, left, right));
                }
            }
            if (operators->top < 0) { *error = 1; break; }
            pop_node(operators);
            prev_tok = tok;
        }
        else if (tok.type == TOKEN_OPERATOR) {
            char cur_op = tok.text[0];
            int is_unary = (prev_tok.type == TOKEN_END || prev_tok.type == TOKEN_LPAREN ||
                           (prev_tok.type == TOKEN_OPERATOR && prev_tok.text[0] != ')'));
            if (is_unary && (cur_op == '+' || cur_op == '-')) {
                push_node(operators, (AstNode*)(intptr_t)(-(cur_op)));
            } else {
                int prec_cur = get_precedence(cur_op);
                while (operators->top >= 0) {
                    intptr_t top_val = (intptr_t)top_node(operators);
                    if (top_val == '(') break;
                    int is_top_unary = (top_val < 0);
                    char top_op = is_top_unary ? -top_val : (char)top_val;
                    int prec_top = get_precedence(top_op);
                    if (prec_top > prec_cur || (prec_top == prec_cur && !is_right_assoc(cur_op))) {
                        pop_node(operators);
                        if (is_top_unary) {
                            AstNode* operand = pop_node(operands);
                            if (!operand) { *error = 1; break; }
                            push_node(operands, create_unary_node(top_op, operand));
                        } else {
                            AstNode* right = pop_node(operands);
                            AstNode* left = pop_node(operands);
                            if (!left || !right) { *error = 1; break; }
                            push_node(operands, create_binary_node(top_op, left, right));
                        }
                    } else break;
                }
                push_node(operators, (AstNode*)(intptr_t)cur_op);
            }
            prev_tok = tok;
        }
        else if (tok.type == TOKEN_ERROR) *error = 1;
    }
    while (operators->top >= 0 && !*error) {
        intptr_t op_val = (intptr_t)pop_node(operators);
        if (op_val == '(') { *error = 1; break; }
        if (op_val < 0) {
            char uop = -op_val;
            AstNode* operand = pop_node(operands);
            if (!operand) { *error = 1; break; }
            push_node(operands, create_unary_node(uop, operand));
        } else {
            char bop = (char)op_val;
            AstNode* right = pop_node(operands);
            AstNode* left = pop_node(operands);
            if (!left || !right) { *error = 1; break; }
            push_node(operands, create_binary_node(bop, left, right));
        }
    }
    AstNode* root = NULL;
    if (!*error && operands->top == 0) root = pop_node(operands);
    else *error = 1;
    if (*error) while (operands->top >= 0) free_ast(pop_node(operands));
    free_stack(operands); free_stack(operators);
    return root;
}

static int is_binary_op_token(const char* tok) { return strlen(tok) == 1 && strchr("+-*/%^#", tok[0]) != NULL; }
static AstNode* parse_prefix_rec(Token* tokens, int* pos, int* error) {
    Token tok = tokens[*pos];
    if (tok.type == TOKEN_END) { *error = 1; return NULL; }
    (*pos)++;
    if (tok.type == TOKEN_NUMBER) return create_number_node(tok.value);
    if (tok.type == TOKEN_VARIABLE) return create_variable_node(tok.text);
    if (tok.type == TOKEN_OPERATOR && is_binary_op_token(tok.text)) {
        AstNode* left = parse_prefix_rec(tokens, pos, error);
        if (*error) return NULL;
        AstNode* right = parse_prefix_rec(tokens, pos, error);
        if (*error) { free_ast(left); return NULL; }
        return create_binary_node(tok.text[0], left, right);
    }
    *error = 1; return NULL;
}
AstNode* parse_prefix(const char* expr, int* error) {
    lexer_init(expr);
    Token tokens[1024]; int count = 0; Token t;
    while ((t = lexer_next()).type != TOKEN_END && count < 1023) tokens[count++] = t;
    tokens[count].type = TOKEN_END;
    int pos = 0;
    AstNode* root = parse_prefix_rec(tokens, &pos, error);
    if (!*error && pos != count) *error = 1;
    if (*error && root) { free_ast(root); root = NULL; }
    return root;
}

typedef struct { AstNode** arr; int top; int cap; } Stack;
static Stack* create_stack_pst(int cap) {
    Stack* s = (Stack*)MALLOC(sizeof(Stack));
    s->arr = (AstNode**)MALLOC(cap * sizeof(AstNode*));
    s->top = -1; s->cap = cap;
    return s;
}
static void push_pst(Stack* s, AstNode* node) { s->arr[++s->top] = node; }
static AstNode* pop_pst(Stack* s) { return (s->top >= 0) ? s->arr[s->top--] : NULL; }
static void free_stack_pst(Stack* s) { FREE(s->arr); FREE(s); }

AstNode* parse_postfix(const char* expr, int* error) {
    lexer_init(expr);
    Stack* st = create_stack_pst(256);
    *error = 0;
    Token tok;
    while ((tok = lexer_next()).type != TOKEN_END && !*error) {
        if (tok.type == TOKEN_NUMBER) push_pst(st, create_number_node(tok.value));
        else if (tok.type == TOKEN_VARIABLE) push_pst(st, create_variable_node(tok.text));
        else if (tok.type == TOKEN_OPERATOR && is_binary_op_token(tok.text)) {
            if ((tok.text[0] == '+' || tok.text[0] == '-') && st->top == 0) {
                AstNode* operand = pop_pst(st);
                if (!operand) { *error = 1; break; }
                push_pst(st, create_unary_node(tok.text[0], operand));
            } else {
                AstNode* right = pop_pst(st);
                AstNode* left = pop_pst(st);
                if (!left || !right) { *error = 1; break; }
                push_pst(st, create_binary_node(tok.text[0], left, right));
            }
        } else *error = 1;
    }
    AstNode* root = NULL;
    if (!*error && st->top == 0) root = pop_pst(st);
    else *error = 1;
    if (*error) while (st->top >= 0) free_ast(pop_pst(st));
    free_stack_pst(st);
    return root;
}
