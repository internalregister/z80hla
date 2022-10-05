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

#ifndef __Z80HLA_H__
#define __Z80HLA_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

// #define DEBUG 1

#define Z80HLA_VERSION_HI   "1"
#define Z80HLA_VERSION_LO   "1"

#define INCLUDE_STACK_MAX   10

#define INCLUDE_ERROR_OVER_MAX_STACK    1
#define INCLUDE_ERROR_CYCLIC            2

#define BOOL char
#define TRUE 1
#define FALSE 0

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

void write_error(char *fmt, ...);
void write_compiler_error(char *filename, int current_line, char *fmt, ...);
void write_debug_impl(char *fmt, ...);

BOOL is_str_equal(char *str1, int str1_size, char *str2);
BOOL is_str_equal2(char *str1, int str1_size, char *str2, int str2_size);
BOOL is_native_type(char *type, int type_size);
int get_native_type_size(char *type, int type_size);
void filename_get_path(char *dst, char *filename);
void filename_add_path(char *dst, char *filename, char *path);

#if DEBUG == 1
#define write_debug(fmt, ...) write_debug_impl(fmt, __VA_ARGS__)
#else
#define write_debug(fmt, ...)
#endif

enum CPUType
{
    CPU_TYPE_Z80,
    CPU_TYPE_GB,
    CPU_TYPE_MSX
};

extern enum CPUType cpu_type, initial_cpu_type;

extern FILE *fp_list;

struct Lexer
{    
    char *buffer_start;
    char *buffer_at;
    char *filename;
    int current_line;
};

enum TokenType
{
    TOKEN_TYPE_INVALID,
    TOKEN_TYPE_END,
    TOKEN_TYPE_LPAREN,
    TOKEN_TYPE_RPAREN,
    TOKEN_TYPE_LCURLY,
    TOKEN_TYPE_RCURLY,
    TOKEN_TYPE_LSQUARE,
    TOKEN_TYPE_RSQUARE,
    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_MINUS,
    TOKEN_TYPE_ASTERISK,
    TOKEN_TYPE_SLASH,
    TOKEN_TYPE_MODULUS,
    TOKEN_TYPE_AMPERSAND,
    TOKEN_TYPE_PIPE,
    TOKEN_TYPE_TILDE,
    TOKEN_TYPE_CIRCUMFLEX,
    TOKEN_TYPE_LEFT_SHIFT,
    TOKEN_TYPE_RIGHT_SHIFT,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_EQUALS,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_IDENTIFIER_OF_LIBRARY,
    TOKEN_TYPE_LABEL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_NEWLINE,
    TOKEN_TYPE_REGISTER,
    TOKEN_TYPE_COND,
    TOKEN_TYPE_OP,
    TOKEN_TYPE_IF,
    TOKEN_TYPE_ELSE,
    TOKEN_TYPE_FUNCTION,
    TOKEN_TYPE_INTERRUPT,
    TOKEN_TYPE_DATA,
    TOKEN_TYPE_DATA_TYPE,
    TOKEN_TYPE_INCLUDE,
    TOKEN_TYPE_PRINT,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_CHARACTER,
    TOKEN_TYPE_LIBRARY,
    TOKEN_TYPE_CONST,
    TOKEN_TYPE_STRUCT,
    TOKEN_TYPE_UNION,
    TOKEN_TYPE_DOLLAR,
    TOKEN_TYPE_DOT,
    TOKEN_TYPE_INLINE,
    TOKEN_TYPE_ORIGIN,
    TOKEN_TYPE_WHILE,
    TOKEN_TYPE_DO,
    TOKEN_TYPE_FOREVER,
    TOKEN_TYPE_BREAK,
    TOKEN_TYPE_BREAKIF,
    TOKEN_TYPE_SIZEOF,
    TOKEN_TYPE_LENGTH,
    TOKEN_TYPE_OUTPUT_ON,
    TOKEN_TYPE_OUTPUT_OFF,
    TOKEN_TYPE_INCLUDE_BINARY,
    TOKEN_TYPE_FROM,
    TOKEN_TYPE_SET_OUTPUT_FILE,
    TOKEN_TYPE_SET_CPU_TYPE,
    TOKEN_TYPE_IFDEF,
    TOKEN_TYPE_IFNDEF,
    TOKEN_TYPE_IFDEF_ELSE,
    TOKEN_TYPE_IFDEF_ENDIF,
    TOKEN_TYPE_DEFINE,
    TOKEN_TYPE_ASSEMBLEALL_ON,
    TOKEN_TYPE_ASSEMBLEALL_OFF,
    TOKEN_TYPE_OF
};

struct Token
{
    enum TokenType type;

    char *value;
    int size;
    int64_t number_value;
    char *value2;
    int size2;
};

enum NodeType
{
    NODE_TYPE_MAIN,
    NODE_TYPE_LABEL,
    NODE_TYPE_OP,
    NODE_TYPE_REGISTER,
    NODE_TYPE_COND,
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_LPAREN,
    NODE_TYPE_INDEX_REGISTER,
    NODE_TYPE_EXPRESSION_8,
    NODE_TYPE_EXPRESSION_8c,
    NODE_TYPE_EXPRESSION_3,
    NODE_TYPE_EXPRESSION_16,
    NODE_TYPE_EXPRESSION_32,
    NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS,
    NODE_TYPE_MATCH_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_INTERRUPT,
    NODE_TYPE_INCLUDE,
    NODE_TYPE_LIBRARY,
    NODE_TYPE_DATA,
    NODE_TYPE_DATA_TYPE,
    NODE_TYPE_DATA_VALUE,
    NODE_TYPE_DATA_SIZE,
    NODE_TYPE_DATA_FROM,
    NODE_TYPE_CONST,
    NODE_TYPE_STRUCT,
    NODE_TYPE_STRUCT_ELEMENT,
    NODE_TYPE_STRUCT_ELEMENT_TYPE,
    NODE_TYPE_UNION,
    NODE_TYPE_PRINT,
    NODE_TYPE_STRUCT_INIT,
    NODE_TYPE_STRUCT_INIT_ELEMENT,
    NODE_TYPE_STRUCT_INIT_ELEMENT_VALUE,
    NODE_TYPE_INDEX,
    NODE_TYPE_FUNCTION_CALL,
    NODE_TYPE_ORIGIN,
    NODE_TYPE_IF,
    NODE_TYPE_WHILE,
    NODE_TYPE_DO,
    NODE_TYPE_FOREVER,
    NODE_TYPE_BREAK,
    NODE_TYPE_BREAKIF,
    NODE_TYPE_STRING,
    NODE_TYPE_OUTPUT_ON,
    NODE_TYPE_OUTPUT_OFF,
    NODE_TYPE_INCLUDE_BINARY,
    NODE_TYPE_SET_OUTPUT_FILE,
    NODE_TYPE_SET_CPU_TYPE,
    NODE_TYPE_ASSEMBLEALL_ON,
    NODE_TYPE_ASSEMBLEALL_OFF    
};

#define MAX_AST_NODE_CHILDREN   16

struct ASTNode
{
    enum NodeType type;
    struct ASTNode *children[MAX_AST_NODE_CHILDREN];
    int children_count;

    char *str_value;
    int str_size;    
    int64_t num_value, num_value2;
    char *str_value2;
    int str_size2;
    
    char *filename;
    int file_line;
};

BOOL is_node_expression_type(enum NodeType node_type);

// Lexer

int init_lexer(struct Lexer *lexer, char *filename);
int destroy_lexer(struct Lexer *lexer);
int get_next_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline);
int peek_next_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline);

// Parser

struct ASTNode *create_node(enum NodeType type, struct Lexer *lexer);
struct ASTNode *create_node_str(enum NodeType type, struct Lexer *lexer, char *str_value, int str_size);
struct ASTNode *create_node_str2(enum NodeType type, struct Lexer *lexer, char *str_value, int str_size, char *str_value2, int str_size2);
struct ASTNode *create_node_num(enum NodeType type, struct Lexer *lexer, int64_t value);
struct ASTNode *create_node_num2(enum NodeType type, struct Lexer *lexer, int64_t value, int64_t value2);
struct ASTNode *duplicate_node(struct ASTNode *node_to_duplicate);
struct ASTNode *duplicate_node_deep(struct ASTNode *node_to_duplicate);
BOOL is_node_expression(struct ASTNode *node);
void fprint_ast(FILE *fp, struct ASTNode *node);
struct ASTNode *parse(struct Lexer *lexer, struct ASTNode *parent_node, struct ASTNode **last_node);

// Tables

void fprint_constants(FILE *fp);
int set_constant(char *library_name, int library_size, char *name, int name_size, int64_t value);
int get_constant(char *library_name, int library_size, char *name, int name_size, int64_t *value);
BOOL is_constant_present(char *library_name, int library_size, char *name, int name_size);

int push_include_file(char *filename);
void pop_include_file();

char *get_content_from_cache(char *filename);
void add_content_to_cache(char *filename, char *content);

void add_library_symbol_dependency(char *library_name, int library_size, char *symbol_name, int symbol_name_size,
	char *library_dependency_name, int library_dependency_name_size, char *symbol_dependency_name, int symbol_dependency_name_size);
void fprint_library_symbol_dependencies(FILE *fp);    
void add_library_symbol_used(char *library_name, int library_size, char *symbol_name, int symbol_name_size);
void fprint_library_symbols_used(FILE *fp);
void fill_library_symbols_used_with_dependencies();
BOOL is_library_symbol_needed(char *library_name, int library_size, char *symbol_name, int symbol_name_size);

void fprintf_output_symbols(FILE *fp);

enum StructuredTypeType { STRUCT_TYPE_STRUCT, STRUCT_TYPE_UNION };

struct StructuredType;

struct StructElement
{
	char *name;
	int name_size;

	char *type;
	int type_size;
    char *type_library;
    int type_library_size;    
    BOOL is_native_type;
    struct StructuredType *structured_type;    

    int size_of_type;
    int array_length;

    int position;

	struct StructElement *next;
};

struct StructuredType
{
	char *name;
	int name_size;

    char *library_name;
    int library_name_size;

    int struct_size;

	enum StructuredTypeType type;

	struct StructElement *first_element;

	struct StructuredType *next;
};

struct StructuredType *create_structured_type(char *name, int name_size, char *library_name, int library_name_size, enum StructuredTypeType type);
struct StructuredType *add_structured_type(struct StructuredType *new_type);
int add_element_to_structured_type(struct StructuredType *structured_type, char *name, int name_size, char *type, int type_size, char *type_library, int type_library_size, int array_length);
struct StructuredType *get_structured_type(char *name, int name_size, char *library_name, int library_name_size);
void fprint_structured_types(FILE *fp);
struct StructElement *get_struct_element(struct StructuredType *structured_type, char *name, int name_size);

extern struct ASTNode **struct_bytes;

void clear_struct_bytes(int size);
int set_struct_bytes(struct StructuredType *structured_type, int offset, struct StructElement *struct_element, int index, struct ASTNode *node);

struct DataSymbol
{
	char *name;
	int name_size;
	char *library_name;
	int library_name_size;

	char *type;
	int type_size;
	char *library_type;
	int library_type_size;

	BOOL is_native_type;
	struct StructuredType *structured_type;

	int size_of_type;
	int length;

	struct DataSymbol *next;
};

int add_data_symbol(char *name, int name_size, char *library_name, int library_name_size, char *type, int type_size, char *library_type, int library_type_size, int length);
struct DataSymbol *get_data_symbol(char *name, int name_size, char *library_name, int library_name_size);
struct DataSymbol *get_last_data_symbol();
void fprintf_data_symbols(FILE *fp);

int add_inline_symbol(char *name, int name_size, char *library_name, int library_name_size, struct ASTNode *node);
struct ASTNode *get_inline_symbol_node(char *name, int name_size, char *library_name, int library_name_size);

void push_loop_label(char *label);
char *peek_loop_label();
void pop_loop_label();

void push_inline_symbol_stack(char *inline_symbol, int inline_symbol_size, char *library_name, int library_name_size);
BOOL is_inline_symbol_in_stack(char *name, int name_size, char *library_name, int library_name_size);
void pop_inline_symbol_stack();

void add_include_path(char *path);
BOOL get_file_include_path(char *output_file_path, char *file_path, char* origin_file_path);

BOOL has_define_identifier(char *identifier, int identifier_size);
void add_define_identifier(char *identifier, int identifier_size);

enum IfdefExpectType
{
    IFDEF_EXPECT_UNKNOWN,
    IFDEF_EXPECT_ELSE,
    IFDEF_EXPECT_ENDIF
};

void push_ifdef_expect(int type);
enum IfdefExpectType peek_ifdef_expect();
void pop_ifdef_expect();
void save_duplicate_all_ifdef_expect();
void revert_to_duplicate_ifdef_expect();

// Compiler

extern int compiler_current_address;
extern char *compiler_output_filename;

extern BOOL assemble_all;

void init_compiler();
int compile(struct ASTNode *first_node);
void fprint_output(FILE *fp);
void add_output_element(uint8_t value, struct ASTNode *node);
void add_output_element_set_address(uint8_t value, struct ASTNode *node);
void dump_output(FILE *fp);

// Ops

int compile_op(struct ASTNode *node, BOOL add_to_output, int *length);
void fprint_operand_node(FILE *fp, struct ASTNode *node);
void fprint_op(FILE *fp, struct ASTNode *node);

// Expression

int resolve_expression(struct ASTNode *node, int64_t *result);

#endif
