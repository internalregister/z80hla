/*
    Copyright (c) 2022, Sérgio Vieira <internalregister@gmail.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "z80hla.h"

static BOOL in_library = FALSE;
static char *current_library_name = NULL;
static int current_library_name_size = 0;
static BOOL in_symbol = FALSE;
static char *current_symbol_name = NULL;
static int current_symbol_name_size = 0;

struct ASTNode *create_node(enum NodeType type, struct Lexer *lexer)
{
    struct ASTNode *node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
    node->type = type;
    node->children[0] = NULL;
    node->children[1] = NULL;
    node->children[2] = NULL;
    node->children_count = 0;
    node->str_size = 0;
    node->str_size2 = 0;
    node->num_value = 0;
    node->num_value2 = 0;
    if (lexer != NULL)
    {
        node->filename = lexer->filename;
        node->file_line = lexer->current_line;
    }
    else
    {
        node->filename = NULL;
        node->file_line = 0;
    }
    return node;
}

struct ASTNode *create_node_str(enum NodeType type, struct Lexer *lexer, char *str_value, int str_size)
{
    struct ASTNode *node = create_node(type, lexer);
    node->str_value = str_value;
    node->str_size = str_size;
    node->str_size2 = 0;
    return node;
}

struct ASTNode *create_node_str2(enum NodeType type, struct Lexer *lexer, char *str_value, int str_size, char *str_value2, int str_size2)
{
    struct ASTNode *node = create_node(type, lexer);
    node->str_value = str_value;
    node->str_size = str_size;
    node->str_value2 = str_value2;
    node->str_size2 = str_size2;
    return node;
}

struct ASTNode *create_node_num(enum NodeType type, struct Lexer *lexer, int64_t value)
{
    struct ASTNode *node = create_node(type, lexer);
    node->str_size = 0;
    node->str_size2 = 0;
    node->num_value = value;
    return node;
}

struct ASTNode *create_node_num2(enum NodeType type, struct Lexer *lexer, int64_t value, int64_t value2)
{
    struct ASTNode *node = create_node(type, lexer);
    node->str_size = 0;
    node->str_size2 = 0;
    node->num_value = value;
    node->num_value2 = value2;
    return node;
}

struct ASTNode *duplicate_node(struct ASTNode *node_to_duplicate)
{
    struct ASTNode *node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
    node->type = node_to_duplicate->type;
    for (int i = 0; i < MAX_AST_NODE_CHILDREN; i++)
    {
        node->children[i] = node_to_duplicate->children[i];
    }
    node->children_count = node_to_duplicate->children_count;
    node->str_value = node_to_duplicate->str_value;
    node->str_value2 = node_to_duplicate->str_value2;
    node->str_size = node_to_duplicate->str_size;
    node->str_size2 = node_to_duplicate->str_size2;
    node->num_value = node_to_duplicate->num_value;
    node->num_value2 = node_to_duplicate->num_value2;
    node->filename = node_to_duplicate->filename;
    node->file_line = node_to_duplicate->file_line;

    return node;
}

struct ASTNode *duplicate_node_deep(struct ASTNode *node_to_duplicate)
{
    assert(node_to_duplicate != NULL);

    struct ASTNode *node = duplicate_node(node_to_duplicate);

    for(int i = 0; i < node->children_count; i++)
    {
        if (node->children[i] != NULL)
        {
            node->children[i] = duplicate_node_deep(node->children[i]);
        }
    }

    return node;
}

BOOL is_node_expression(struct ASTNode *node)
{
    if (node != NULL)
    {
        if (node->type == NODE_TYPE_EXPRESSION ||
            node->type == NODE_TYPE_EXPRESSION_3 ||
            node->type == NODE_TYPE_EXPRESSION_8 ||
            node->type == NODE_TYPE_EXPRESSION_8c ||
            node->type == NODE_TYPE_EXPRESSION_16 ||
            node->type == NODE_TYPE_EXPRESSION_32 ||
            node->type == NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS)
        {
            return 1;
        }
    }

    return FALSE;
}

char *node_type_names[] = {
    "NODE_TYPE_MAIN",
    "NODE_TYPE_LABEL",
    "NODE_TYPE_OP",
    "NODE_TYPE_REGISTER",
    "NODE_TYPE_COND",
    "NODE_TYPE_EXPRESSION",
    "NODE_TYPE_LPAREN",
    "NODE_TYPE_INDEX_REGISTER",
    "NODE_TYPE_EXPRESSION_8",
    "NODE_TYPE_EXPRESSION_8c",
    "NODE_TYPE_EXPRESSION_3",
    "NODE_TYPE_EXPRESSION_16",
    "NODE_TYPE_EXPRESSION_32",
    "NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS",
    "NODE_TYPE_MATCH_LIST",
    "NODE_TYPE_FUNCTION",
    "NODE_TYPE_INTERRUPT",
    "NODE_TYPE_INCLUDE",
    "NODE_TYPE_LIBRARY",
    "NODE_TYPE_DATA",
    "NODE_TYPE_DATA_TYPE",
    "NODE_TYPE_DATA_VALUE",
    "NODE_TYPE_DATA_SIZE",
    "NODE_TYPE_DATA_FROM",
    "NODE_TYPE_CONST",
    "NODE_TYPE_STRUCT",
    "NODE_TYPE_STRUCT_ELEMENT",
    "NODE_TYPE_STRUCT_ELEMENT_TYPE",
    "NODE_TYPE_UNION",
    "NODE_TYPE_PRINT",
    "NODE_TYPE_STRUCT_INIT",
    "NODE_TYPE_STRUCT_INIT_ELEMENT",
    "NODE_TYPE_STRUCT_INIT_ELEMENT_VALUE",
    "NODE_TYPE_INDEX",
    "NODE_TYPE_FUNCTION_CALL",
    "NODE_TYPE_ORIGIN",
    "NODE_TYPE_IF",
    "NODE_TYPE_WHILE",
    "NODE_TYPE_DO",
    "NODE_TYPE_FOREVER",
    "NODE_TYPE_BREAK",
    "NODE_TYPE_BREAKIF",
    "NODE_TYPE_STRING",
    "NODE_TYPE_OUTPUT_ON",
    "NODE_TYPE_OUTPUT_OFF",
    "NODE_TYPE_INCLUDE_BINARY",
    "NODE_TYPE_SET_OUTPUT_FILE",
    "NODE_TYPE_SET_CPU_TYPE"
};

void fprint_ast(FILE *fp, struct ASTNode *node)
{
    if (node == NULL)
    {
        fprintf(fp, "null");
        return;
    }

    fprintf(fp, "{\"type\": \"%s\",", node_type_names[node->type]);
    fprintf(fp, "\"filename\": \"%s\",", (node->filename == NULL ? "(N/A)" : node->filename));
    fprintf(fp, "\"file_line\": \"%d\",", node->file_line);
    fprintf(fp, "\"str_value\": \"%.*s\",", node->str_size, node->str_value);
    fprintf(fp, "\"str_value2\": \"%.*s\",", node->str_size2, node->str_value2);
    fprintf(fp, "\"num_value\": %"PRId64",", node->num_value);
    fprintf(fp, "\"num_value2\": %"PRId64",", node->num_value2);
    fprintf(fp, "\"children_count\": %d,", node->children_count);
    fprintf(fp, "\"children\": [");
    for (int i = 0; i < node->children_count; i++)
    {
        if (i > 0)
        {
            fprintf(fp, ",");
        }
        fprint_ast(fp, node->children[i]);
    }
    fprintf(fp, "]}");
}

static void write_error_unexpected_token(struct Lexer *lexer, struct Token *token)
{
    write_compiler_error(lexer->filename, lexer->current_line, "Unexpected token \"%.*s\"", token->size, token->value);
}

static struct ASTNode *parse_expression(struct Lexer *lexer);

static int parse_expression_index_identifier(struct Lexer *lexer, struct ASTNode **out_expression_index_node)
{
    struct ASTNode *expression_index_node = NULL;
    struct Token token;

    *out_expression_index_node = NULL;

    if (peek_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LSQUARE)
    {
        return 0;
    }
    if (get_next_token(lexer, &token, TRUE)) return 1;

    expression_index_node = create_node(NODE_TYPE_INDEX, lexer);

    struct ASTNode *expression_node = parse_expression(lexer);
    if (expression_node == NULL)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected expression for indexing value, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    expression_index_node->children[0] = expression_node;
    expression_index_node->children_count = 1;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_RSQUARE)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"]\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    *out_expression_index_node = expression_index_node;

    return 0;
}

static int parse_expression_identifier(struct Lexer *lexer, struct ASTNode **out_expression_node, BOOL allow_indexes)
{
    struct ASTNode *expression_node = NULL, *expression_index_node = NULL;
    struct Token token;

    if (peek_next_token(lexer, &token, TRUE)) return 1;
    switch(token.type)
    {
        case TOKEN_TYPE_IDENTIFIER:
        {
            if (get_next_token(lexer, &token, TRUE)) return 1;
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            if (in_library)
            {
                expression_node->str_value2 = current_library_name;
                expression_node->str_size2 = current_library_name_size;
            }

            if (in_library && in_symbol)
            {
                add_library_symbol_dependency(
                    current_library_name, current_library_name_size,
                    current_symbol_name, current_symbol_name_size,
                    current_library_name, current_library_name_size,
                    token.value, token.size);
            }

            break;
        }
        case TOKEN_TYPE_IDENTIFIER_OF_LIBRARY:
        {
            if (get_next_token(lexer, &token, TRUE)) return 1;
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value2 = token.value;
            expression_node->str_size2 = token.size;
            expression_node->str_value = token.value2;
            expression_node->str_size = token.size2;

            if (in_library && in_symbol)
            {
                add_library_symbol_dependency(
                    current_library_name, current_library_name_size,
                    current_symbol_name, current_symbol_name_size,
                    token.value, token.size,
                    token.value2, token.size2);
            }
            else
            {
                add_library_symbol_used(
                    token.value, token.size,
                    token.value2, token.size2);
            }

            break;
        }
        default:
        {
            return 0;
        }
    }

    if (allow_indexes)
    {
        if (parse_expression_index_identifier(lexer, &expression_index_node)) { return 1; }
        if (expression_index_node != NULL)
        {
            expression_node->children[0] = expression_index_node;
            expression_node->children_count = 1;
        }
    }

    if (peek_next_token(lexer, &token, TRUE)) return 1;
    struct ASTNode *last_identifier_node = expression_node, *last_parent_node = NULL;
    while(token.type == TOKEN_TYPE_DOT)
    {
        if (get_next_token(lexer, &token, TRUE)) return 1;

        struct ASTNode *new_expression_node = create_node_str(NODE_TYPE_EXPRESSION, lexer, token.value, token.size);
        new_expression_node->children[0] = last_identifier_node;

        if (get_next_token(lexer, &token, TRUE)) return 1;
        if (token.type != TOKEN_TYPE_IDENTIFIER)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier after \".\", found \"%.*s\"", token.size, token.value);
            return 1;
        }

        struct ASTNode *new_identifier_node = create_node_str(NODE_TYPE_EXPRESSION, lexer, token.value, token.size);

        if (allow_indexes)
        {
            if (parse_expression_index_identifier(lexer, &expression_index_node)) { return 1; }
            if (expression_index_node != NULL)
            {
                new_identifier_node->children[0] = expression_index_node;
                new_identifier_node->children_count = 1;
            }
        }

        new_expression_node->children[1] = new_identifier_node;
        new_expression_node->children_count = 2;

        if (last_parent_node == NULL)
        {
            expression_node = new_expression_node;
        }
        else
        {
            last_parent_node->children[1] = new_expression_node;
        }

        last_identifier_node = new_identifier_node;
        last_parent_node = new_expression_node;

        if (peek_next_token(lexer, &token, TRUE)) return 1;
    }

    *out_expression_node = expression_node;

    return 0;
}

static struct ASTNode *parse_bitwise_or_expression(struct Lexer *lexer);

static struct ASTNode *parse_primary_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL;
    struct Token token;

    if (parse_expression_identifier(lexer, &expression_node, TRUE)) { return NULL; }
    if (expression_node != NULL)
    {
        return expression_node;
    }

    if (get_next_token(lexer, &token, TRUE)) return NULL;
    switch(token.type)
    {
        case TOKEN_TYPE_NUMBER:
        {
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_size = 0;
            expression_node->num_value = token.number_value;
            break;
        }
        case TOKEN_TYPE_DOLLAR:
        {
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_size = 1;
            expression_node->str_value = token.value;
            expression_node->num_value = 0;
            break;
        }
        case TOKEN_TYPE_LPAREN:
        {
            expression_node = parse_bitwise_or_expression(lexer);
            expression_node->num_value = -1;
            if (expression_node == NULL) { return NULL; }
            if (get_next_token(lexer, &token, TRUE)) return NULL;
            if (token.type != TOKEN_TYPE_RPAREN)
            {
                write_error_unexpected_token(lexer, &token);
                return NULL;
            }
            break;
        }
        case TOKEN_TYPE_SIZEOF:
        {
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_size = token.size;
            expression_node->str_value = token.value;
            expression_node->num_value = 0;

            if (get_next_token(lexer, &token, TRUE)) return NULL;
            if (token.type != TOKEN_TYPE_LPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in sizeof expression, found \"%.*s\"", token.size, token.value);
                return NULL;
            }

            struct ASTNode *identifier_node = NULL;
            if (parse_expression_identifier(lexer, &identifier_node, FALSE)) { return NULL; }

            if (identifier_node == NULL)
            {
                if (get_next_token(lexer, &token, TRUE)) return NULL;
                if (token.type != TOKEN_TYPE_DATA_TYPE)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier or type in sizeof expression", 0);
                    return NULL;
                }
                identifier_node = create_node_str(NODE_TYPE_DATA_TYPE, lexer, token.value, token.size);
            }

            expression_node->children_count = 1;
            expression_node->children[0] = identifier_node;

            if (get_next_token(lexer, &token, TRUE)) return NULL;
            if (token.type != TOKEN_TYPE_RPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" in sizeof expression, found \"%.*s\"", token.size, token.value);
                return NULL;
            }

            break;
        }
        case TOKEN_TYPE_LENGTH:
        {
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_size = token.size;
            expression_node->str_value = token.value;
            expression_node->num_value = 0;

            if (get_next_token(lexer, &token, TRUE)) return NULL;
            if (token.type != TOKEN_TYPE_LPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in length expression, found \"%.*s\"", token.size, token.value);
                return NULL;
            }

            struct ASTNode *identifier_node = NULL;
            if (parse_expression_identifier(lexer, &identifier_node, FALSE)) { return NULL; }

            if (identifier_node == NULL)
            {
                if (get_next_token(lexer, &token, TRUE)) return NULL;
                if (token.type != TOKEN_TYPE_DATA_TYPE)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier or type in length expression", 0);
                    return NULL;
                }
                identifier_node = create_node_str(NODE_TYPE_DATA_TYPE, lexer, token.value, token.size);
            }

            expression_node->children_count = 1;
            expression_node->children[0] = identifier_node;

            if (get_next_token(lexer, &token, TRUE)) return NULL;
            if (token.type != TOKEN_TYPE_RPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" in length expression, found \"%.*s\"", token.size, token.value);
                return NULL;
            }

            break;
        }
        
        case TOKEN_TYPE_CHARACTER:
        {
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_size = 0;
            expression_node->num_value = token.number_value;
            break;
        }
        default:
            write_compiler_error(lexer->filename, lexer->current_line, "Unexpected token \"%.*s\" in expression", token.size, token.value);
            return NULL;
    }

    return expression_node;
}

static struct ASTNode *parse_unary_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL;
    struct Token token;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_PLUS:
        case TOKEN_TYPE_MINUS:
        case TOKEN_TYPE_TILDE:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *pri_expression_node = parse_primary_expression(lexer);
            if (pri_expression_node == NULL)
            {
                return NULL;
            }

            expression_node->children[0] = pri_expression_node;
            expression_node->children_count = 1;

            break;
        }
        default:
            expression_node = parse_primary_expression(lexer);
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_multiplicative_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *unary_expression_node = NULL;
    struct Token token;

    unary_expression_node = parse_unary_expression(lexer);
    if (unary_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_ASTERISK:
        case TOKEN_TYPE_SLASH:
        case TOKEN_TYPE_MODULUS:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *mul_expression_node = parse_multiplicative_expression(lexer);
            if (mul_expression_node == NULL)
            {
                return NULL;
            }

            if (mul_expression_node->num_value != -1 &&
                (is_str_equal(mul_expression_node->str_value, mul_expression_node->str_size, "*") ||
                is_str_equal(mul_expression_node->str_value, mul_expression_node->str_size, "/") ||
                is_str_equal(mul_expression_node->str_value, mul_expression_node->str_size, "%")))
            {
                struct ASTNode *node = mul_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 &&
                    (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "*") ||
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "/") ||
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "%")))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = unary_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = mul_expression_node;
            }
            else
            {
                expression_node->children[0] = unary_expression_node;
                expression_node->children[1] = mul_expression_node;
                expression_node->children_count = 2;
            }

            break;
        }
        default:
            expression_node = unary_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_additive_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *mul_expression_node = NULL;
    struct Token token;

    mul_expression_node = parse_multiplicative_expression(lexer);
    if (mul_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_PLUS:
        case TOKEN_TYPE_MINUS:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *add_expression_node = parse_additive_expression(lexer);
            if (add_expression_node == NULL) { return NULL; }

            if (add_expression_node->num_value != -1 && add_expression_node->children_count == 2 &&
                (is_str_equal(add_expression_node->str_value, add_expression_node->str_size, "+") ||
                is_str_equal(add_expression_node->str_value, add_expression_node->str_size, "-")))
            {
                struct ASTNode *node = add_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 && node->children[0]->children_count == 2 &&
                    (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "+") ||
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "-")))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = mul_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = add_expression_node;
            }
            else
            {
                expression_node->children[0] = mul_expression_node;
                expression_node->children[1] = add_expression_node;
                expression_node->children_count = 2;
            }            

            break;
        }
        default:
            expression_node = mul_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_shift_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *add_expression_node = NULL;
    struct Token token;

    add_expression_node = parse_additive_expression(lexer);
    if (add_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_LEFT_SHIFT:
        case TOKEN_TYPE_RIGHT_SHIFT:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *shift_expression_node = parse_shift_expression(lexer);
            if (shift_expression_node == NULL) { return NULL; }

            if (shift_expression_node->num_value != -1 && shift_expression_node->children_count == 2 &&
                (is_str_equal(shift_expression_node->str_value, shift_expression_node->str_size, "<<") ||
                is_str_equal(shift_expression_node->str_value, shift_expression_node->str_size, ">>")))
            {
                struct ASTNode *node = shift_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 && node->children[0]->children_count == 2 &&
                    (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "<<") ||
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, ">>")))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = add_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = shift_expression_node;
            }
            else
            {
                expression_node->children[0] = add_expression_node;
                expression_node->children[1] = shift_expression_node;
                expression_node->children_count = 2;
            }            

            break;
        }
        default:
            expression_node = add_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_bitwise_and_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *shift_expression_node = NULL;
    struct Token token;

    shift_expression_node = parse_shift_expression(lexer);
    if (shift_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_AMPERSAND:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *bitwise_expression_node = parse_bitwise_and_expression(lexer);
            if (bitwise_expression_node == NULL)
            {
                return NULL;
            }

            if (bitwise_expression_node->num_value != -1 &&
                is_str_equal(bitwise_expression_node->str_value, bitwise_expression_node->str_size, "&"))
            {
                struct ASTNode *node = bitwise_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 &&
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "&"))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = shift_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = bitwise_expression_node;
            }
            else
            {
                expression_node->children[0] = shift_expression_node;
                expression_node->children[1] = bitwise_expression_node;
                expression_node->children_count = 2;
            }

            break;
        }
        default:
            expression_node = shift_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_bitwise_xor_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *bitwise_and_expression_node = NULL;
    struct Token token;

    bitwise_and_expression_node = parse_bitwise_and_expression(lexer);
    if (bitwise_and_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_CIRCUMFLEX:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *bitwise_expression_node = parse_bitwise_xor_expression(lexer);
            if (bitwise_expression_node == NULL)
            {
                return NULL;
            }

            if (bitwise_expression_node->num_value != -1 &&
                is_str_equal(bitwise_expression_node->str_value, bitwise_expression_node->str_size, "^"))
            {
                struct ASTNode *node = bitwise_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 &&
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "^"))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = bitwise_and_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = bitwise_expression_node;
            }
            else
            {
                expression_node->children[0] = bitwise_and_expression_node;
                expression_node->children[1] = bitwise_expression_node;
                expression_node->children_count = 2;
            }

            break;
        }
        default:
            expression_node = bitwise_and_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_bitwise_or_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL, *bitwise_xor_expression_node = NULL;
    struct Token token;

    bitwise_xor_expression_node = parse_bitwise_xor_expression(lexer);
    if (bitwise_xor_expression_node == NULL) return NULL;

    peek_next_token(lexer, &token, TRUE);
    switch(token.type)
    {
        case TOKEN_TYPE_PIPE:
        {
            get_next_token(lexer, &token, TRUE);
            expression_node = create_node(NODE_TYPE_EXPRESSION, lexer);
            expression_node->str_value = token.value;
            expression_node->str_size = token.size;

            struct ASTNode *bitwise_expression_node = parse_bitwise_or_expression(lexer);
            if (bitwise_expression_node == NULL)
            {
                return NULL;
            }

            if (bitwise_expression_node->num_value != -1 &&
                is_str_equal(bitwise_expression_node->str_value, bitwise_expression_node->str_size, "|"))
            {
                struct ASTNode *node = bitwise_expression_node;
                while(node->children[0] != NULL && node->children[0]->num_value != -1 &&
                    is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "|"))
                {
                    node = node->children[0];
                }
                expression_node->children[0] = bitwise_xor_expression_node;
                expression_node->children[1] = node->children[0];
                expression_node->children_count = 2;
                node->children[0] = expression_node;
                expression_node = bitwise_expression_node;
            }
            else
            {
                expression_node->children[0] = bitwise_xor_expression_node;
                expression_node->children[1] = bitwise_expression_node;
                expression_node->children_count = 2;
            }

            break;
        }
        default:
            expression_node = bitwise_xor_expression_node;
            break;
    }

    return expression_node;
}

static struct ASTNode *parse_expression(struct Lexer *lexer)
{
    struct ASTNode *expression_node = NULL;

    expression_node = parse_bitwise_or_expression(lexer);

    return expression_node;
}

static int parse_op_operand(struct Lexer *lexer, struct ASTNode *op_node)
{
    struct Token token;
    struct ASTNode *operand_node;
    if (peek_next_token(lexer, &token, TRUE)) return 1;

    switch(token.type)
    {
        case TOKEN_TYPE_LPAREN:
        {
            get_next_token(lexer, &token, TRUE);
            operand_node = create_node(NODE_TYPE_LPAREN, lexer);
            peek_next_token(lexer, &token, TRUE);
            switch (token.type)
            {
                case TOKEN_TYPE_REGISTER:
                {
                    get_next_token(lexer, &token, TRUE);
                    struct ASTNode *register_node = create_node(NODE_TYPE_REGISTER, lexer);
                    register_node->str_value = token.value;
                    register_node->str_size = token.size;

                    // Check for index registers
                    peek_next_token(lexer, &token, TRUE);
                    if (token.type == TOKEN_TYPE_PLUS || token.type == TOKEN_TYPE_MINUS)
                    {
                        // (register +/- expression)                    
                        struct ASTNode *index_register_node = create_node(NODE_TYPE_INDEX_REGISTER, lexer);                        

                        index_register_node->children_count = 2;
                        index_register_node->children[0] = register_node;
                        index_register_node->children[1] = parse_expression(lexer);
                        if (index_register_node->children[1] == NULL)
                        {
                            return 1;
                        }

                        operand_node->children_count = 1;
                        operand_node->children[0] = index_register_node;
                    }
                    else
                    {
                        // (register)                
                        operand_node->children_count = 1;
                        operand_node->children[0] = register_node;
                    }
                    
                    break;
                }
                default:
                {
                    // (expression)
                    struct ASTNode *expression_node = parse_expression(lexer);
                    if (expression_node != NULL)
                    {
                        operand_node->children_count = 1;
                        operand_node->children[0] = expression_node;
                    }
                    else
                    {
                        write_error_unexpected_token(lexer, &token);
                        return 1;
                    }
                }
            }
            get_next_token(lexer, &token, TRUE);
            if (token.type != TOKEN_TYPE_RPAREN)
            {
                write_error_unexpected_token(lexer, &token);
                return 1;
            }
            break;
        }
        case TOKEN_TYPE_REGISTER:
        {
            get_next_token(lexer, &token, TRUE);
            operand_node = create_node(NODE_TYPE_REGISTER, lexer);
            operand_node->str_value = token.value;
            operand_node->str_size = token.size;
            break;
        }
        case TOKEN_TYPE_COND:
        {
            get_next_token(lexer, &token, TRUE);
            operand_node = create_node(NODE_TYPE_COND, lexer);
            operand_node->str_value = token.value;
            operand_node->str_size = token.size;
            break;
        }
        default:
        {
            operand_node = parse_expression(lexer);
            if (operand_node == NULL)
            {
                write_error_unexpected_token(lexer, &token);
                return 1;
            }
        }
    }
     
    op_node->children[op_node->children_count] = operand_node;
    op_node->children_count++;    

    return 0;
}

static int parse_op(struct Lexer *lexer, struct ASTNode *op_node)
{
    struct Token token;
    if (peek_next_token(lexer, &token, FALSE)) return 1;
    if (token.type == TOKEN_TYPE_NEWLINE)
    {
        // No operands
        return 0;
    }

    // First operand
    if (parse_op_operand(lexer, op_node)) return 1;

    if (peek_next_token(lexer, &token, TRUE)) return 1;
    while (token.type == TOKEN_TYPE_COMMA)
    {
        if (op_node->children_count == MAX_AST_NODE_CHILDREN)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Maximum number of operands exceeded (%d)", MAX_AST_NODE_CHILDREN);
            return 1;
        }

        // More operands
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        if (parse_op_operand(lexer, op_node)) return 1;

        if (peek_next_token(lexer, &token, TRUE)) return 1;
    }

    return 0;
}

static int parse_block(struct Lexer *lexer, struct ASTNode *parent_node, struct Token *token, struct ASTNode **out_ast_node);

static int parse_while(struct Lexer *lexer, struct ASTNode *while_node)
{
    struct Token token;
    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in while statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_COND && (token.type != TOKEN_TYPE_REGISTER || !is_str_equal(token.value, token.size, "c")))
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected condition at while statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }
    while_node->str_value = token.value;
    while_node->str_size = token.size;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_RPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" after condition in while statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\" in while statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);

    while_node->children_count = 1;
    while_node->children[0] = ast_node_main;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected block statement or \"}\" in while statement, found \"%.*s\"", token.size, token.value);
                return 1;
            }

            break;
        }
    } while(TRUE);    

    return 0;
}

static int parse_do(struct Lexer *lexer, struct ASTNode *do_node)
{
    struct Token token;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\" in do statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);

    do_node->children_count = 1;
    do_node->children[0] = ast_node_main;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected block statement or \"}\" in do statement, found \"%.*s\"", token.size, token.value);
                return 1;
            }

            break;
        }
    } while(TRUE);  

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_WHILE)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"while\" in do statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in do statement after \"while\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_COND && (token.type != TOKEN_TYPE_REGISTER || !is_str_equal(token.value, token.size, "c")))
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected condition at do statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }
    do_node->str_value = token.value;
    do_node->str_size = token.size;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_RPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" after condition in do statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }  

    return 0;
}

static int parse_forever(struct Lexer *lexer, struct ASTNode *forever_node)
{
    struct Token token;
    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\" in forever statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);

    forever_node->children_count = 1;
    forever_node->children[0] = ast_node_main;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected block statement or \"}\" in forever statement, found \"%.*s\"", token.size, token.value);
                return 1;
            }

            break;
        }
    } while(TRUE);    

    return 0;
}

static int parse_if(struct Lexer *lexer, struct ASTNode *if_node)
{
    struct Token token;
    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in if statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_COND && (token.type != TOKEN_TYPE_REGISTER || !is_str_equal(token.value, token.size, "c")))
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected condition at if statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }
    if_node->str_value = token.value;
    if_node->str_size = token.size;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_RPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" after condition in if statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\" in if statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);

    if_node->children_count = 1;
    if_node->children[0] = ast_node_main;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected block statement or \"}\" in if statement, found \"%.*s\"", token.size, token.value);
                return 1;
            }

            break;
        }
    } while(TRUE);

    if (peek_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_ELSE)
    {
        return 0;
    }
    if (get_next_token(lexer, &token, TRUE)) return 1;

    if (get_next_token(lexer, &token, TRUE)) return 1;
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\" in else statement, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main2 = create_node(NODE_TYPE_MAIN, lexer);

    if_node->children_count = 2;
    if_node->children[1] = ast_node_main2;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main2, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main2 = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected block statement or \"}\" in else statement, found \"%.*s\"", token.size, token.value);
                return 1;
            }

            return 0;
        }
    } while(TRUE);

    return 0;
}

static int parse_block(struct Lexer *lexer, struct ASTNode *parent_node, struct Token *token, struct ASTNode **out_ast_node)
{
    struct ASTNode *ast_node = NULL;

    *out_ast_node = NULL;

    switch(token->type)
    {
        case TOKEN_TYPE_LABEL:
        {
            ast_node = create_node(NODE_TYPE_LABEL, lexer);
            ast_node->str_value = token->value;
            ast_node->str_size = token->size;

            if (in_library)
            {
                ast_node->str_value2 = current_library_name;
                ast_node->str_size2 = current_library_name_size;

                add_library_symbol_dependency(
                    current_library_name, current_library_name_size,
                    token->value, token->size,
                    current_library_name, current_library_name_size,
                    current_symbol_name, current_symbol_name_size);
            }

            break;
        }
        case TOKEN_TYPE_OP:
        {
            ast_node = create_node(NODE_TYPE_OP, lexer);
            ast_node->str_value = token->value;
            ast_node->str_size = token->size;
            
            if (parse_op(lexer, ast_node)) return 1;

            break;
        }
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_IDENTIFIER_OF_LIBRARY:
        {
            ast_node = create_node(NODE_TYPE_FUNCTION_CALL, lexer);

            if (token->size2 > 0)
            {
                ast_node->str_value = token->value2;
                ast_node->str_size = token->size2;
                ast_node->str_value2 = token->value;
                ast_node->str_size2 = token->size;
            }
            else
            {
                ast_node->str_value = token->value;
                ast_node->str_size = token->size;

                if (in_library)
                {
                    ast_node->str_value2 = current_library_name;
                    ast_node->str_size2 = current_library_name_size;
                }
            }

            if (in_library && in_symbol)
            {
                if (token->size2 > 0)
                {
                    add_library_symbol_dependency(
                        current_library_name, current_library_name_size,
                        current_symbol_name, current_symbol_name_size,
                        token->value, token->size,
                        token->value2, token->size2);
                }
                else
                {
                    add_library_symbol_dependency(
                        current_library_name, current_library_name_size,
                        current_symbol_name, current_symbol_name_size,
                        current_library_name, current_library_name_size,
                        token->value, token->size);
                }
            }
            else if (token->size2 > 0)
            {
                add_library_symbol_used(
                    token->value, token->size,
                    token->value2, token->size2);
            }

            struct Token inner_token;
            if (get_next_token(lexer, &inner_token, FALSE)) return 1;
            if (inner_token.type != TOKEN_TYPE_LPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" after identifier for function call, found \"%.*s\"", inner_token.size, inner_token.value);
                return 1;
            }

            if (get_next_token(lexer, &inner_token, FALSE)) return 1;
            if (inner_token.type != TOKEN_TYPE_RPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" in function call, found \"%.*s\"", inner_token.size, inner_token.value);
                return 1;
            }

            break;
        }
        case TOKEN_TYPE_IF:
        {
            ast_node = create_node(NODE_TYPE_IF, lexer);
            if (parse_if(lexer, ast_node)) return 1;
            
            break;
        }
        case TOKEN_TYPE_WHILE:
        {
            ast_node = create_node(NODE_TYPE_WHILE, lexer);
            if (parse_while(lexer, ast_node)) return 1;

            break;
        }
        case TOKEN_TYPE_DO:
        {
            ast_node = create_node(NODE_TYPE_DO, lexer);
            if (parse_do(lexer, ast_node)) return 1;

            break;
        }
        case TOKEN_TYPE_FOREVER:
        {
            ast_node = create_node(NODE_TYPE_FOREVER, lexer);
            if (parse_forever(lexer, ast_node)) return 1;

            break;
        }
        case TOKEN_TYPE_BREAK:
        {
            ast_node = create_node(NODE_TYPE_BREAK, lexer);

            break;
        }
        case TOKEN_TYPE_BREAKIF:
        {
            ast_node = create_node(NODE_TYPE_BREAKIF, lexer);

            struct Token inner_token;

            if (get_next_token(lexer, &inner_token, TRUE)) return 1;
            if (inner_token.type != TOKEN_TYPE_LPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\" in breakif statement, found \"%.*s\"", inner_token.size, inner_token.value);
                return 1;
            }

            if (get_next_token(lexer, &inner_token, TRUE)) return 1;
            if (inner_token.type != TOKEN_TYPE_COND && (inner_token.type != TOKEN_TYPE_REGISTER || !is_str_equal(inner_token.value, inner_token.size, "c")))
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected condition in breakif statement, found \"%.*s\"", inner_token.size, inner_token.value);
                return 1;
            }
            ast_node->str_value = inner_token.value;
            ast_node->str_size = inner_token.size;

            if (get_next_token(lexer, &inner_token, TRUE)) return 1;
            if (inner_token.type != TOKEN_TYPE_RPAREN)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\" after condition in breakif statement, found \"%.*s\"", inner_token.size, inner_token.value);
                return 1;
            }

            break;
        }
        case TOKEN_TYPE_PRINT:
        {
            ast_node = create_node(NODE_TYPE_PRINT, lexer);            
            struct Token inner_token;

            struct ASTNode *last_data_node = NULL, *data_node = NULL;

            do
            {
                if (peek_next_token(lexer, &inner_token, FALSE)) return 1;
                if (inner_token.type == TOKEN_TYPE_STRING)
                {
                    if (get_next_token(lexer, &inner_token, FALSE)) return 1;
                    data_node = create_node(NODE_TYPE_DATA_VALUE, lexer);
                    data_node->children_count = 2;
                    data_node->str_value = inner_token.value;
                    data_node->str_size = inner_token.size;                    
                }
                else
                {                    
                    struct ASTNode *expression_node = parse_expression(lexer);
                    if (expression_node == NULL)
                    {
                        write_compiler_error(lexer->filename, lexer->current_line, "Expected string or expression for #print", 0);
                        return 1;
                    }
                    data_node = create_node(NODE_TYPE_DATA_VALUE, lexer);
                    data_node->children_count = 2;
                    data_node->children[0] = expression_node;
                }

                if (last_data_node == NULL)
                {
                    ast_node->children_count = 1;
                    ast_node->children[0] = data_node;
                }
                else
                {
                    last_data_node->children[1] = data_node;
                }
                last_data_node = data_node;

                if (get_next_token(lexer, &inner_token, FALSE)) return 1;
                if (inner_token.type == TOKEN_TYPE_NEWLINE)
                {                    
                    break;
                }
                else if (inner_token.type != TOKEN_TYPE_COMMA)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected new line or \",\"", 0);
                    return 1;
                }

            } while(TRUE);
            
            break;
        }
        default:
            return 0;
    }

    parent_node->children[0] = ast_node;
    parent_node->children[1] = create_node(NODE_TYPE_MAIN, lexer);
    parent_node->children_count = 2;
    parent_node = parent_node->children[1];

    *out_ast_node = parent_node;

    return 0;
}

static int parse_inline(struct Lexer *lexer)
{
    struct Token token;

    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_IDENTIFIER)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);
    
    if (!in_library)
    {
        add_inline_symbol(token.value, token.size, "", 0, ast_node_main);
    }
    else
    {
        add_inline_symbol(token.value, token.size, current_library_name, current_library_name_size, ast_node_main);
    }

    in_symbol = TRUE;
    current_symbol_name = token.value;
    current_symbol_name_size = token.size;

    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_LPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\", found \"%.*s\"", token.size, token.value);
        return 1;
    }
    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_RPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\", found \"%.*s\"", token.size, token.value);
        return 1;
    }
    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_error_unexpected_token(lexer, &token);
                return 1;
            }

            in_symbol = FALSE;
            return 0;
        }
    } while(TRUE);

    return 1;
}

static int parse_function(struct Lexer *lexer, struct ASTNode *function_node)
{
    struct Token token;

    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_IDENTIFIER)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier, found \"%.*s\"", token.size, token.value);
        return 1;
    }
    function_node->str_size = token.size;
    function_node->str_value = token.value;
    
    if (in_library)
    {
        function_node->str_value2 = current_library_name;
        function_node->str_size2 = current_library_name_size;
    }

    in_symbol = TRUE;
    current_symbol_name = token.value;
    current_symbol_name_size = token.size;

    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_LPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"(\", found \"%.*s\"", token.size, token.value);
        return 1;
    }
    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_RPAREN)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \")\", found \"%.*s\"", token.size, token.value);
        return 1;
    }
    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *ast_node_main = create_node(NODE_TYPE_MAIN, lexer);

    function_node->children_count = 1;
    function_node->children[0] = ast_node_main;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        struct ASTNode *temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return 1; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_error_unexpected_token(lexer, &token);
                return 1;
            }

            in_symbol = FALSE;
            return 0;
        }
    } while(TRUE);

    return 1;
}

static struct ASTNode *parse_struct_init(struct Lexer *lexer)
{
    struct Token token;

    if (peek_next_token(lexer, &token, TRUE)) { return NULL; }
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        return NULL;
    }    
    if (get_next_token(lexer, &token, TRUE)) { return NULL; }

    struct ASTNode *struct_init_node = create_node(NODE_TYPE_STRUCT_INIT, lexer);
    struct ASTNode *last_struct_init_element = NULL;

    do
    {
        if (get_next_token(lexer, &token, TRUE)) { return NULL; }
        if (token.type == TOKEN_TYPE_RCURLY)
        {
            break;
        }

        if (token.type != TOKEN_TYPE_IDENTIFIER)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Expected name of parameter, found \"%.*s\"", token.size, token.value);
            return NULL;
        }

        struct ASTNode *struct_init_element = create_node_str(NODE_TYPE_STRUCT_INIT_ELEMENT, lexer, token.value, token.size);

        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
        if (token.type != TOKEN_TYPE_EQUALS)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Expected \"=\", found \"%.*s\"", token.size, token.value);
            return NULL;
        }        

        if (last_struct_init_element == NULL)
        {
            struct_init_node->children[0] = struct_init_element;
            struct_init_node->children_count = 1;
        }
        else
        {
            last_struct_init_element->children[1] = struct_init_element;
            last_struct_init_element->children_count = 2;            
        }

        last_struct_init_element = struct_init_element;

        struct ASTNode *last_struct_init_element_value = NULL;

        do
        {
            if (peek_next_token(lexer, &token, TRUE)) { return NULL; }
            struct ASTNode *value_node = NULL;
            if (token.type == TOKEN_TYPE_LCURLY)
            {
                // struct init
                value_node = parse_struct_init(lexer);
                if (value_node == NULL)
                {
                    return NULL;
                }
            }
            else
            {
                // expression
                value_node = parse_expression(lexer);
                if (value_node == NULL)
                {
                    return NULL;
                }
            }

            struct ASTNode *struct_init_element_value = create_node(NODE_TYPE_STRUCT_INIT_ELEMENT_VALUE, lexer);
            struct_init_element_value->children[0] = value_node;
            struct_init_element_value->children_count = 1;

            if (last_struct_init_element_value == NULL)
            {
                last_struct_init_element->children[0] = struct_init_element_value;
                last_struct_init_element->children_count = 1;
            }
            else
            {
                last_struct_init_element_value->children[1] = struct_init_element_value;
                last_struct_init_element_value->children_count = 2;
            }

            last_struct_init_element_value = struct_init_element_value;

            if (get_next_token(lexer, &token, FALSE)) { return NULL; }
            if (token.type == TOKEN_TYPE_NEWLINE)
            {
                break;
            }
            else if (token.type == TOKEN_TYPE_RCURLY)
            {
                return struct_init_node;
            }
            else if (token.type != TOKEN_TYPE_COMMA)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \",\" or new line in structure initializer, found \"%.*s\"", token.size, token.value);
                return NULL;
            }
        } while (TRUE);        

    } while (TRUE);

    return struct_init_node;
}

static int parse_data(struct Lexer *lexer, struct ASTNode *data_node)
{
    struct Token token;

    if (get_next_token(lexer, &token, FALSE)) { return 1; }
    else if (token.type != TOKEN_TYPE_IDENTIFIER && token.type != TOKEN_TYPE_IDENTIFIER_OF_LIBRARY && token.type != TOKEN_TYPE_DATA_TYPE)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected data type, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    // data type    
    struct ASTNode *data_type_node = NULL;
    if (token.size2 > 0)
    {
        data_type_node = create_node_str2(NODE_TYPE_DATA_TYPE, lexer, token.value2, token.size2, token.value, token.size);
    }
    else
    {
        if (in_library && !is_native_type(token.value, token.size))
        {            
            data_type_node = create_node_str2(NODE_TYPE_DATA_TYPE, lexer, token.value, token.size, current_library_name, current_library_name_size);
        }
        else
        {
            data_type_node = create_node_str(NODE_TYPE_DATA_TYPE, lexer, token.value, token.size);
        }
    }
    data_node->children[0] = data_type_node;

    data_node->str_size = 0;

    if (get_next_token(lexer, &token, FALSE)) { return 1; }
    if (token.type == TOKEN_TYPE_IDENTIFIER)
    {
        data_node->str_value = token.value;
        data_node->str_size = token.size;

        if (in_library)
        {
            data_node->str_value2 = current_library_name;
            data_node->str_size2 = current_library_name_size;
        }

        in_symbol = TRUE;
        current_symbol_name = token.value;
        current_symbol_name_size = token.size;

        if (get_next_token(lexer, &token, FALSE)) { return 1; }
    }
    else if (in_library)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier in data declarations in a library, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    if (token.type == TOKEN_TYPE_EQUALS)
    {
        // Initialized data
        struct ASTNode *expression_node = NULL, *data_value_node = NULL, *last_data_value_node = NULL;
        do
        {
            if (peek_next_token(lexer, &token, FALSE)) { return 1; }
            if (token.type == TOKEN_TYPE_LCURLY)
            {
                expression_node = parse_struct_init(lexer);
                if (expression_node == NULL) { return 1; }
            }
            else if (token.type == TOKEN_TYPE_STRING)
            {
                expression_node = create_node_str(NODE_TYPE_STRING, lexer, token.value, token.size);
                get_next_token(lexer, &token, FALSE);
            }
            else
            {
                expression_node = parse_expression(lexer);
                if (expression_node == NULL)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected expression, found \"%.*s\"", token.size, token.value);
                    return 1;
                }
            }            

            data_value_node = create_node(NODE_TYPE_DATA_VALUE, lexer);
            data_value_node->children_count = 2;
            data_value_node->children[0] = expression_node;
            data_value_node->children[1] = NULL;

            if (last_data_value_node != NULL)
            {
                last_data_value_node->children[1] = data_value_node;
            }
            else
            {
                data_node->children[1] = data_value_node;
            }
            last_data_value_node = data_value_node;

            if (get_next_token(lexer, &token, FALSE)) { return 1; }
            if (token.type == TOKEN_TYPE_NEWLINE)
            {
                break;
            }
            if (token.type != TOKEN_TYPE_COMMA)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected comma or new line, found \"%.*s\"", token.size, token.value);
                return 1;
            }
        } while(TRUE);
    }
    else if (token.type == TOKEN_TYPE_LSQUARE || token.type == TOKEN_TYPE_NEWLINE)
    {
        BOOL size_defined = (token.type == TOKEN_TYPE_LSQUARE);
        // Uninitialized data
        struct ASTNode *expression_node = NULL;
        if (size_defined)
        {
            expression_node = parse_expression(lexer);
            if (expression_node == NULL)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected expression, found \"%.*s\"", token.size, token.value);
                return 1;
            }
        }
        else
        {
            expression_node = create_node_num(NODE_TYPE_EXPRESSION, lexer, 1);
        }

        struct ASTNode *data_size_node = create_node(NODE_TYPE_DATA_SIZE, lexer);
        data_size_node->children_count = 1;
        data_size_node->children[0] = expression_node;
        data_node->children[1] = data_size_node;

        if (size_defined)
        {
            if (get_next_token(lexer, &token, FALSE)) { return 1; }
            if (token.type != TOKEN_TYPE_RSQUARE)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected \"]\", found \"%.*s\"", token.size, token.value);
                return 1;
            }

            if (get_next_token(lexer, &token, FALSE)) { return 1; }
            if (token.type != TOKEN_TYPE_NEWLINE)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected new line, found \"%.*s\"", token.size, token.value);
                return 1;
            }
        }
    }
    else if (token.type == TOKEN_TYPE_FROM)
    {
        if (get_next_token(lexer, &token, FALSE)) { return 1; }
        if (token.type != TOKEN_TYPE_STRING)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Expected string literal after \"from\", found \"%.*s\"", token.size, token.value);
            return 1;
        }

        struct ASTNode *data_from_node = create_node_str(NODE_TYPE_DATA_FROM, lexer, token.value, token.size);
        data_node->children[1] = data_from_node;

        if (get_next_token(lexer, &token, FALSE)) { return 1; }
        if (token.type != TOKEN_TYPE_NEWLINE)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line, found \"%.*s\"", token.size, token.value);
            return 1;
        }
    }
    else
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"=\" or \"[\" or \"from\", found \"%.*s\"", token.size, token.value);
        return 1;
    }
    
    data_node->children_count = 2;

    in_symbol = FALSE;

    return 0;
}

static int parse_const(struct Lexer *lexer, struct ASTNode *const_node)
{
    struct Token token;

    if (get_next_token(lexer, &token, FALSE)) { return 1; }
    if (token.type != TOKEN_TYPE_IDENTIFIER)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    const_node->str_value = token.value;
    const_node->str_size = token.size;

    if (in_library)
    {
        const_node->str_value2 = current_library_name;
        const_node->str_size2 = current_library_name_size;
    }

    in_symbol = TRUE;
    current_symbol_name = token.value;
    current_symbol_name_size = token.size;

    if (get_next_token(lexer, &token, FALSE)) { return 1; }
    if (token.type != TOKEN_TYPE_EQUALS)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"=\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    struct ASTNode *expression_node = parse_expression(lexer);
    if (expression_node == NULL)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected expression, found \"%.*s\"", token.size, token.value);
        return 1;
    }

    const_node->children[0] = expression_node;
    const_node->children_count = 1;

    in_symbol = FALSE;

    return 0;
}

static int parse_struct(struct Lexer *lexer, struct ASTNode *struct_node, BOOL with_name);

// Anything that can be declared inside libraries and outside as well
static struct ASTNode *parse_declarations(struct Lexer *lexer, struct ASTNode *parent_node, struct Token *token, BOOL *empty)
{
    *empty = FALSE;
    switch(token->type)
    {
        case TOKEN_TYPE_FUNCTION:
        {            
            struct ASTNode *ast_node = create_node(NODE_TYPE_FUNCTION, lexer);
            if (parse_function(lexer, ast_node)) { return NULL; }
            
            parent_node->children[0] = ast_node;
            parent_node->children[1] = create_node(NODE_TYPE_MAIN, lexer);
            parent_node->children_count = 2;
            parent_node = parent_node->children[1];

            return parent_node;
        }
        case TOKEN_TYPE_DATA:
        {
            struct ASTNode *ast_node = create_node(NODE_TYPE_DATA, lexer);
            if (parse_data(lexer, ast_node)) { return NULL; }

            parent_node->children[0] = ast_node;
            parent_node->children[1] = create_node(NODE_TYPE_MAIN, lexer);
            parent_node->children_count = 2;
            parent_node = parent_node->children[1];
            
            return parent_node;
        }
        case TOKEN_TYPE_CONST:
        {
            struct ASTNode *ast_node = create_node(NODE_TYPE_CONST, lexer);
            if (parse_const(lexer, ast_node)) { return NULL; }

            parent_node->children[0] = ast_node;
            parent_node->children[1] = create_node(NODE_TYPE_MAIN, lexer);
            parent_node->children_count = 2;
            parent_node = parent_node->children[1];
            
            return parent_node;
        }
        case TOKEN_TYPE_STRUCT:
        case TOKEN_TYPE_UNION:
        {
            struct ASTNode *ast_node = create_node(token->type == TOKEN_TYPE_STRUCT ? NODE_TYPE_STRUCT : NODE_TYPE_UNION, lexer);
            if (parse_struct(lexer, ast_node, TRUE)) { return NULL; }

            parent_node->children[0] = ast_node;
            parent_node->children[1] = create_node(NODE_TYPE_MAIN, lexer);
            parent_node->children_count = 2;
            parent_node = parent_node->children[1];
            
            return parent_node;
        }
        case TOKEN_TYPE_INLINE:
        {
            if (parse_inline(lexer)) { return NULL; }

            return parent_node;
        }
        default:
            break;
    }

    *empty = TRUE;
    return NULL;
}

static struct ASTNode *parse_library(struct Lexer *lexer, struct ASTNode *ast_node_main)
{
    struct Token token;    

    if (get_next_token(lexer, &token, TRUE)) { return NULL; }
    if (token.type != TOKEN_TYPE_IDENTIFIER)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier, found \"%.*s\"", token.size, token.value);
        return NULL;
    }

    in_library = TRUE;
    current_library_name = token.value;
    current_library_name_size = token.size;
    
    if (get_next_token(lexer, &token, TRUE)) { return NULL; }
    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\", found \"%.*s\"", token.size, token.value);
        return NULL;
    }

    do {
        if (get_next_token(lexer, &token, TRUE)) { return NULL; }
        BOOL empty = FALSE;
        struct ASTNode *temp_node = parse_declarations(lexer, ast_node_main, &token, &empty);
        if (empty == FALSE)
        {
            if (temp_node == NULL)
            {
                return NULL;
            }
            ast_node_main = temp_node;
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_error_unexpected_token(lexer, &token);
                return NULL;
            }

            in_library = FALSE;
            return ast_node_main;
        }
    } while(TRUE);    

    return NULL;
}

// Parse struct and union
static int parse_struct(struct Lexer *lexer, struct ASTNode *struct_node, BOOL with_name)
{
    struct Token token;

    if (get_next_token(lexer, &token, TRUE)) { return 1; }
    if (token.type == TOKEN_TYPE_IDENTIFIER)
    {
        if (!with_name)
        {
            write_compiler_error(lexer->filename, lexer->current_line, "anonymous structured type expected");
            return 1;
        }

        struct_node->str_size = token.size;
        struct_node->str_value = token.value;

        if (in_library)
        {
            struct_node->str_value2 = current_library_name;
            struct_node->str_size2 = current_library_name_size;
        }

        if (get_next_token(lexer, &token, TRUE)) { return 1; }
    }
    else if (with_name)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "name needed from structured type");
        return 1;
    }

    if (token.type != TOKEN_TYPE_LCURLY)
    {
        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"{\", found \"%.*s\"", token.size, token.value);
        return 1;
    }

    int element_count = 0;

    struct ASTNode *last_element_node = NULL;
    struct ASTNode *last_element_type_node = NULL;

    do {
        if (get_next_token(lexer, &token, TRUE)) { return 1; }
        if (token.type == TOKEN_TYPE_DATA_TYPE || token.type == TOKEN_TYPE_IDENTIFIER || token.type == TOKEN_TYPE_IDENTIFIER_OF_LIBRARY ||
            token.type == TOKEN_TYPE_STRUCT || token.type == TOKEN_TYPE_UNION)
        {
            struct ASTNode *element_type_node = create_node(NODE_TYPE_STRUCT_ELEMENT_TYPE, lexer);

            if (token.type == TOKEN_TYPE_DATA_TYPE || token.type == TOKEN_TYPE_IDENTIFIER)
            {
                element_type_node->str_size = token.size;
                element_type_node->str_value = token.value;
            }
            else if (token.type == TOKEN_TYPE_IDENTIFIER_OF_LIBRARY)
            {
                element_type_node->str_size = token.size2;
                element_type_node->str_value = token.value2;
                element_type_node->str_size2 = token.size;
                element_type_node->str_value2 = token.value;
            }
            else if (token.type == TOKEN_TYPE_STRUCT || token.type == TOKEN_TYPE_UNION)
            {
                element_type_node->str_size = token.size;
                element_type_node->str_value = token.value;
                struct ASTNode *inner_struct_node = create_node(token.type == TOKEN_TYPE_STRUCT ? NODE_TYPE_STRUCT : NODE_TYPE_UNION, lexer);
                element_type_node->children_count = 3;
                if (parse_struct(lexer, inner_struct_node, FALSE)) { return 1; }
                element_type_node->children[0] = NULL;
                element_type_node->children[1] = NULL;
                element_type_node->children[2] = inner_struct_node;
            }

            if (last_element_type_node == NULL)
            {
                struct_node->children_count = 1;
                struct_node->children[0] = element_type_node;                
            }
            else
            {
                last_element_type_node->children[1] = element_type_node;
                if (last_element_type_node->children_count < 2) last_element_type_node->children_count = 2;
            }

            last_element_type_node = element_type_node;

            do
            {
                if (get_next_token(lexer, &token, TRUE)) { return 1; }
                if (token.type != TOKEN_TYPE_IDENTIFIER)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected identifier for element name, found \"%.*s\"", token.size, token.value);
                    return 1;
                }

                struct ASTNode *element_node = create_node(NODE_TYPE_STRUCT_ELEMENT, lexer);
                element_node->str_size = token.size;
                element_node->str_value = token.value;

                if (peek_next_token(lexer, &token, FALSE)) { return 1; }
                if (token.type == TOKEN_TYPE_LSQUARE)
                {
                    // array
                    if (get_next_token(lexer, &token, FALSE)) { return 1; }
                    struct ASTNode *exp_node = parse_expression(lexer);
                    if (exp_node == NULL)
                    {
                        return 1;
                    }
                    element_node->children[1] = exp_node;
                    element_node->children_count = 2;
                    if (get_next_token(lexer, &token, FALSE)) { return 1; }
                    if (token.type != TOKEN_TYPE_RSQUARE)
                    {
                        write_compiler_error(lexer->filename, lexer->current_line, "Expected \"]\", found \"%.*s\"", token.size, token.value);
                    }
                }

                if (last_element_node == NULL)
                {
                    last_element_type_node->children[0] = element_node;
                    if (last_element_type_node->children_count < 1) last_element_type_node->children_count++;
                }
                else
                {
                    if (last_element_node->children_count < 2) last_element_node->children_count = 1;
                    last_element_node->children[0] = element_node;
                }
                last_element_node = element_node;
                element_count++;                
                
                if (peek_next_token(lexer, &token, FALSE)) { return 1; }
                if (token.type == TOKEN_TYPE_NEWLINE)
                {
                    last_element_node = NULL;
                    break;
                }
                else if (token.type != TOKEN_TYPE_COMMA)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Expected \",\" or new line, found \"%.*s\"", token.size, token.value);
                    return 1;
                }
                else
                {
                    if (get_next_token(lexer, &token, FALSE)) { return 1; }
                }
            } while (TRUE);            
        }
        else
        {
            if (token.type != TOKEN_TYPE_RCURLY)
            {
                write_error_unexpected_token(lexer, &token);
                return 1;
            }
            else
            {
                if (element_count == 0)
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Unexpected empty structured type");
                }
            }

            return 0;
        }
    } while(TRUE);

    return 1;
}

struct ASTNode *parse(struct Lexer *lexer, struct ASTNode *parent_node, struct ASTNode **last_node)
{
    struct Token token;
    struct ASTNode *ast_node_main = NULL, *first_node, *temp_node;    

    if (parent_node != NULL)
    {
        ast_node_main = parent_node;
    }
    else
    {
        ast_node_main = create_node(NODE_TYPE_MAIN, lexer);
    }

    first_node = ast_node_main;

    while(!get_next_token(lexer, &token, TRUE))
    {
        temp_node = NULL;
        if (parse_block(lexer, ast_node_main, &token, &temp_node)) { return NULL; }
        if (temp_node != NULL)
        {
            ast_node_main = temp_node;
        }
        else
        {
            BOOL empty = FALSE;
            temp_node = parse_declarations(lexer, ast_node_main, &token, &empty);
            if (!empty)
            {
                if (temp_node == NULL)
                {
                    return NULL;
                }
                ast_node_main = temp_node;
            }
            else
            {
                switch(token.type)
                {                
                    case TOKEN_TYPE_INTERRUPT:
                    {
                        struct ASTNode *ast_node = create_node(NODE_TYPE_INTERRUPT, lexer);
                        if (parse_function(lexer, ast_node)) { return NULL; }
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];
                        
                        break;
                    }
                    case TOKEN_TYPE_INCLUDE:
                    {
                        // Include file

                        if (get_next_token(lexer, &token, TRUE)) { return NULL; }
                        if (token.type == TOKEN_TYPE_STRING)
                        {
                            struct Lexer lexer_include;
                            int include_result;

                            char *new_filename = (char*)malloc(256);
                            if (!get_file_include_path(new_filename, token.value, lexer->filename))
                            {
                                write_compiler_error(lexer->filename, lexer->current_line, "Error including file \"%s\"", token.value);
                                return NULL;
                            }

                            if ((include_result = push_include_file(new_filename)))
                            {
                                switch (include_result)
                                {
                                    case INCLUDE_ERROR_OVER_MAX_STACK:
                                    {
                                        write_compiler_error(lexer->filename, lexer->current_line, "Error including file \"%s\" maximum include level reached", token.value);
                                        break;
                                    }
                                    case INCLUDE_ERROR_CYCLIC:
                                    {
                                        write_compiler_error(lexer->filename, lexer->current_line, "Error including file \"%s\" include cycle detected", token.value);
                                        break;
                                    }
                                    default:
                                    {
                                        assert(1 == 0 && "Unknown include error");
                                    }
                                }
                                return NULL;
                            }

                            if (init_lexer(&lexer_include, new_filename)) // The value in string literal tokens are zero-terminated
                            {
                                write_compiler_error(lexer->filename, lexer->current_line, "Error including file \"%s\"", token.value);
                                return NULL;
                            }                        
                            if (parse(&lexer_include, ast_node_main, &ast_node_main) == NULL)
                            {
                                write_compiler_error(lexer->filename, lexer->current_line, "Error including file \"%s\"", token.value);
                                return NULL;
                            }
                            pop_include_file();
                        }
                        else
                        {
                            write_error_unexpected_token(lexer, &token);
                            return NULL;
                        }
                        break;
                    }
                    case TOKEN_TYPE_ORIGIN:
                    {
                        struct ASTNode *expression_node = parse_expression(lexer);
                        if (expression_node == NULL)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected expression for origin, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line after #origin expression, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node(NODE_TYPE_ORIGIN, lexer);
                        ast_node->children[0] = expression_node;
                        ast_node->children_count = 1;
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_OUTPUT_ON:
                    {
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line after #output_on, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node(NODE_TYPE_OUTPUT_ON, lexer);
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_OUTPUT_OFF:
                    {             
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line after #output_on, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node(NODE_TYPE_OUTPUT_OFF, lexer);
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_INCLUDE_BINARY:
                    {   
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_STRING)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected string in #include_binary statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node_str(NODE_TYPE_INCLUDE_BINARY, lexer, token.value, token.size);
                                  
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line in #include_binary statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_SET_OUTPUT_FILE:
                    {
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_STRING)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected string in #output_file statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node_str(NODE_TYPE_SET_OUTPUT_FILE, lexer, token.value, token.size);
                                  
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line in #output_file statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_SET_CPU_TYPE:
                    {
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_STRING)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected string in #cpu_type statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        int set_cpu_type = (int)CPU_TYPE_Z80;

                        if (is_str_equal(token.value, token.size, "z80"))
                        {
                            set_cpu_type = (int)CPU_TYPE_Z80;
                        }
                        else if (is_str_equal(token.value, token.size, "gb"))
                        {
                            set_cpu_type = (int)CPU_TYPE_GB;
                        }
                        else if (is_str_equal(token.value, token.size, "msx") || is_str_equal(token.value, token.size, "r800"))
                        {
                            set_cpu_type = (int)CPU_TYPE_MSX;
                        }
                        else
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Invalid cpu type \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        struct ASTNode *ast_node = create_node_num(NODE_TYPE_SET_CPU_TYPE, lexer, (int64_t) set_cpu_type);
                                  
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_NEWLINE)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected new line in #cpu_type statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }
                        
                        ast_node_main->children[0] = ast_node;
                        ast_node_main->children[1] = create_node(NODE_TYPE_MAIN, lexer);
                        ast_node_main->children_count = 2;
                        ast_node_main = ast_node_main->children[1];

                        break;
                    }
                    case TOKEN_TYPE_DEFINE:
                    {
                        if (get_next_token(lexer, &token, FALSE)) { return NULL; }
                        if (token.type != TOKEN_TYPE_IDENTIFIER)
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected identifer in #define statement, found \"%.*s\"", token.size, token.value);
                            return NULL;
                        }

                        add_define_identifier(token.value, token.size);

                        break;
                    }
                    case TOKEN_TYPE_LIBRARY:
                    {
                        ast_node_main = parse_library(lexer, ast_node_main);
                        if (ast_node_main == NULL) { return NULL; }                    
                        
                        break;
                    }
                    case TOKEN_TYPE_END:
                    {
                        // End of file

                        if (last_node != NULL)
                        {
                            *last_node = ast_node_main;
                        }
                        return first_node;
                    }
                    default:
                    {
                        write_error_unexpected_token(lexer, &token);
                        write_debug("here", 0);
                        return NULL;
                    }
                }
            }
        }
    }    

    return NULL;
}
