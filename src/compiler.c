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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int output_length = 0;

struct OutputElement
{
    uint8_t value;
    struct ASTNode *node;
    BOOL final; // an element to be written to the output file
    BOOL set_address; // set compiler address at this point

    struct OutputElement *next, *previous;
};

struct OutputElement *first_output_elem, *last_output_elem;

void init_compiler()
{
    first_output_elem = (struct OutputElement *)malloc(sizeof(struct OutputElement));
    first_output_elem->node = NULL;
    first_output_elem->next = NULL;
    first_output_elem->previous = NULL;

    last_output_elem = first_output_elem;

    output_length = 0;
}

static void inner_add_output_element(uint8_t value, struct ASTNode *node, BOOL final, BOOL set_address)
{
    struct OutputElement *elem = first_output_elem;

    if (output_length > 0)
    {
        elem = (struct OutputElement *)malloc(sizeof(struct OutputElement));
        elem->next = NULL;
        elem->previous = last_output_elem;
        last_output_elem->next = elem;

        last_output_elem = elem;
    }

    elem->value = value;
    elem->node = node;
    elem->final = final;
    elem->set_address = set_address;

    output_length++;
}

void add_output_element(uint8_t value, struct ASTNode *node)
{
    inner_add_output_element(value, node, TRUE, FALSE);
}

void add_output_element_set_address(uint8_t value, struct ASTNode *node)
{
    inner_add_output_element(value, node, TRUE, TRUE);
}

static void add_output_element_not_final(struct ASTNode *node)
{
    inner_add_output_element(0, node, FALSE, FALSE);
}

static int struct_count = 0;

static char *get_new_struct_name()
{
    char *name = (char *)malloc(sizeof(char) * 32);
    sprintf(name, "@%d", struct_count++);
    return name;
}

// Convert struct or union ASTNode to structured type
static int build_structured_type(struct ASTNode *struct_node, char **name, int *name_size)
{
    char *struct_name = struct_node->str_value;
    int struct_name_size = struct_node->str_size;

    if (struct_node->str_size == 0)
    {
        struct_name = get_new_struct_name();
        struct_name_size = (int)strlen(struct_name);
    }

    struct StructuredType *structured_type = create_structured_type(struct_name, struct_name_size, struct_node->str_value2, struct_node->str_size2, struct_node->type == NODE_TYPE_STRUCT ? STRUCT_TYPE_STRUCT : STRUCT_TYPE_UNION);
    if (structured_type == NULL)
    {
        if (struct_node->str_size2 > 0)
        {
            write_compiler_error(struct_node->filename, struct_node->file_line, "Symbol name of structured type \"%.*s::%.*s\" already defined", struct_node->str_size2, struct_node->str_value2, struct_node->str_size, struct_node->str_value);
        }
        else
        {
            write_compiler_error(struct_node->filename, struct_node->file_line, "Symbol name of structured type \"%.*s\" already defined", struct_node->str_size, struct_node->str_value);
        }
        return 1;
    }

    write_debug("structured_type %.*s %p", struct_name_size, struct_name, structured_type);

    struct ASTNode *current_element_type_node = struct_node->children[0];    
    while (current_element_type_node != NULL)
    {
        char *element_type_name = current_element_type_node->str_value;
        int element_type_name_size = current_element_type_node->str_size;
        char *element_type_library_name = current_element_type_node->str_value2;
        int element_type_library_name_size = current_element_type_node->str_size2;
        struct ASTNode *current_element_node = current_element_type_node->children[0];

        write_debug("  current_element_type_node %.*s", current_element_type_node->str_size, current_element_type_node->str_value);

        if (current_element_type_node->children_count == 3)
        {
            assert(current_element_type_node->children[2]->type == NODE_TYPE_STRUCT || current_element_type_node->children[2]->type == NODE_TYPE_UNION);            
            build_structured_type(current_element_type_node->children[2], &element_type_name, &element_type_name_size);
            element_type_library_name = "";
            element_type_library_name_size = 0;
        }

        while (current_element_node != NULL)
        {
            write_debug("    current_element_node %.*s", current_element_node->str_size, current_element_node->str_value);

            int64_t array_length = 1;
            if (current_element_node->children_count > 1)
            {
                // second child is an expression for the array length
                resolve_expression(current_element_node->children[1], &array_length);                
            }

            if (add_element_to_structured_type(structured_type,
                current_element_node->str_value, current_element_node->str_size,
                element_type_name, element_type_name_size,
                element_type_library_name, element_type_library_name_size, (int)array_length))
            {
                // may already exist or have an invalid type
                write_compiler_error(struct_node->filename, struct_node->file_line, "Invalid element \"%.*s\" in structured type", current_element_node->str_size, current_element_node->str_value);
                return 1;
            }

            current_element_node = current_element_node->children[0];
        }

        current_element_type_node = current_element_type_node->children[1];
    }

    add_structured_type(structured_type);

    *name = struct_name;
    *name_size = struct_name_size;

    return 0;
}

static void write_print_start(char *filename, int current_line)
{
	printf("[%s:%d] ", filename, current_line);
}

static void write_print_string(char *str, int str_size)
{
	printf("%.*s", str_size, str);
}

static void write_print_expression(int64_t value)
{
	printf("%"PRId64"", value);
}

static void write_print_end()
{
	printf("\n");
}

static int new_label_count = 0;
char *get_new_label()
{
    char *label = (char*)malloc(sizeof(char) * 64);
    sprintf(label, "@l_%d", new_label_count++);
    return label;
}

static struct ASTNode *create_jp_cond_node(char *cond, int cond_size, char *label)
{
    struct ASTNode *jp_node = create_node_str(NODE_TYPE_OP, NULL, "jp", 2);
    struct ASTNode *jp_op1_node = create_node_str(NODE_TYPE_COND, NULL, cond, cond_size);
    if (!strcmp(cond, "c"))
    {
        jp_op1_node->type = NODE_TYPE_REGISTER;
    }
    struct ASTNode *jp_op2_node = create_node_str(NODE_TYPE_EXPRESSION, NULL, label, strlen(label));
    jp_node->children[0] = jp_op1_node;
    jp_node->children[1] = jp_op2_node;
    jp_node->children_count = 2;

    return jp_node;
}

static struct ASTNode *create_jr_cond_node(char *cond, int cond_size, char *label)
{
    struct ASTNode *jp_node = NULL;
    if (is_str_equal(cond, cond_size, "c") || is_str_equal(cond, cond_size, "nc") || is_str_equal(cond, cond_size, "z") || is_str_equal(cond, cond_size, "nz"))
    {
        jp_node = create_node_str(NODE_TYPE_OP, NULL, "jr", 2);
    }    
    else
    {
        jp_node = create_node_str(NODE_TYPE_OP, NULL, "jp", 2);
    }
    struct ASTNode *jp_op1_node = create_node_str(NODE_TYPE_COND, NULL, cond, cond_size);
    if (!strcmp(cond, "c"))
    {
        jp_op1_node->type = NODE_TYPE_REGISTER;
    }
    struct ASTNode *jp_op2_node = create_node_str(NODE_TYPE_EXPRESSION, NULL, label, strlen(label));
    jp_node->children[0] = jp_op1_node;
    jp_node->children[1] = jp_op2_node;
    jp_node->children_count = 2;

    return jp_node;
}

static struct ASTNode *create_reverse_jp_cond_node(char *cond, int cond_size, char *label)
{
    if (is_str_equal(cond, cond_size, "c"))
    {
        return create_jp_cond_node("nc", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "nc"))
    {
        return create_jp_cond_node("c", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "m"))
    {
        return create_jp_cond_node("p", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "p"))
    {
        return create_jp_cond_node("m", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "z"))
    {
        return create_jp_cond_node("nz", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "nz"))
    {
        return create_jp_cond_node("z", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "pe"))
    {
        return create_jp_cond_node("po", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "po"))
    {
        return create_jp_cond_node("pe", 2, label);
    }

    return NULL;
}

static struct ASTNode *create_reverse_jr_cond_node(char *cond, int cond_size, char *label)
{
    if (is_str_equal(cond, cond_size, "c"))
    {
        return create_jr_cond_node("nc", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "nc"))
    {
        return create_jr_cond_node("c", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "m"))
    {
        return create_jr_cond_node("p", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "p"))
    {
        return create_jr_cond_node("m", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "z"))
    {
        return create_jr_cond_node("nz", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "nz"))
    {
        return create_jr_cond_node("z", 1, label);
    }
    else if (is_str_equal(cond, cond_size, "pe"))
    {
        return create_jr_cond_node("po", 2, label);
    }
    else if (is_str_equal(cond, cond_size, "po"))
    {
        return create_jr_cond_node("pe", 2, label);
    }

    return NULL;
}

static struct ASTNode *create_jp_node(char *label)
{
    struct ASTNode *jp_node = create_node_str(NODE_TYPE_OP, NULL, "jp", 2);
    struct ASTNode *jp_op1_node = create_node_str(NODE_TYPE_EXPRESSION, NULL, label, strlen(label));
    jp_node->children[0] = jp_op1_node;
    jp_node->children_count = 1;

    return jp_node;
}

static struct ASTNode *create_jr_node(char *label)
{
    struct ASTNode *jr_node = create_node_str(NODE_TYPE_OP, NULL, "jr", 2);
    struct ASTNode *jr_op1_node = create_node_str(NODE_TYPE_EXPRESSION, NULL, label, strlen(label));
    jr_node->children[0] = jr_op1_node;
    jr_node->children_count = 1;

    return jr_node;
}

static struct ASTNode *add_main_node_to_end(struct ASTNode *child_node, struct ASTNode *last_main_node)
{
    struct ASTNode *new_node = create_node(NODE_TYPE_MAIN, NULL);
    new_node->children_count = 1;
    new_node->children[0] = child_node;

    last_main_node->children_count = 2;
    last_main_node->children[1] = new_node;

    return new_node;
}

static int recursive_first_pass(struct ASTNode *first_node, int *length)
{
    struct ASTNode *current_node = first_node, *node;    

    while (current_node != NULL && current_node->children_count > 0)
    {
        node = current_node->children[0];

        assert(current_node->type == NODE_TYPE_MAIN);

        switch(node->type)
        {
            case NODE_TYPE_OP:
            {
                int op_length = 0;
                if (compile_op(node, FALSE, &op_length))
                {
                    return 1;
                }
                assert(op_length > 0 && op_length < MAX(5, MAX_AST_NODE_CHILDREN+1));
                *length += op_length;
                break;
            }
            case NODE_TYPE_FUNCTION:
            {
                struct ASTNode *last_node = current_node->children[1], *last_cur_node;

                // check if this function is inside a library and needed
                if (node->str_size2 > 0 && !is_library_symbol_needed(node->str_value2, node->str_size2, node->str_value, node->str_size))
                {
                    write_debug("Function \"%.*s::%.*s\" unneeded", node->str_size2, node->str_value2, node->str_size, node->str_value);
                    break;
                }

                struct ASTNode *label_node = create_node_str2(NODE_TYPE_LABEL, NULL, node->str_value, node->str_size, node->str_value2, node->str_size2);
                label_node->filename = node->filename; label_node->file_line = node->file_line;
                current_node->children[0] = label_node;

                last_cur_node = current_node->children[1] = node->children[0];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                struct ASTNode *ret_node = create_node_str(NODE_TYPE_OP, NULL, "ret", 3);
                last_cur_node->children_count = 1;
                last_cur_node->children[0] = ret_node;

                int inner_length = 0;
                if (recursive_first_pass(current_node, &inner_length) ) { return 1; }
                *length += inner_length;

                last_cur_node->children_count = 2;
                last_cur_node->children[1] = last_node;

                current_node = last_cur_node;
                break;
            }
            case NODE_TYPE_INTERRUPT:
            {
                struct ASTNode *last_node = current_node->children[1], *last_cur_node;

                struct ASTNode *label_node = create_node_str(NODE_TYPE_LABEL, NULL, node->str_value, node->str_size);
                label_node->filename = node->filename; label_node->file_line = node->file_line;
                current_node->children[0] = label_node;

                last_cur_node = current_node->children[1] = node->children[0];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }
                
                struct ASTNode *ret_node = create_node_str(NODE_TYPE_OP, NULL, "reti", 4);
                if (is_str_equal(node->str_value, node->str_size, "nmi") && cpu_type != CPU_TYPE_GB)
                {
                    ret_node->str_value = "retn";
                }
                last_cur_node->children_count = 1;
                last_cur_node->children[0] = ret_node;

                int inner_length = 0;
                if (recursive_first_pass(current_node, &inner_length) ) { return 1; }
                *length += inner_length;
                write_debug("Size inside function \"%.*s\" is %d bytes", node->str_size, node->str_value, inner_length);

                last_cur_node->children_count = 2;
                last_cur_node->children[1] = last_node;

                current_node = last_cur_node;
                break;
            }
            case NODE_TYPE_FUNCTION_CALL:
            {                
                struct ASTNode *inline_node = get_inline_symbol_node(node->str_value, node->str_size, node->str_value2, node->str_size2);
                if (inline_node != NULL)
                {
                    if (is_inline_symbol_in_stack(node->str_value, node->str_size, node->str_value2, node->str_size2))
                    {
                        if (node->str_size2 > 0)
                        {
                            write_compiler_error(node->filename, node->file_line, "Cyclic reference of inline \"%.*s::%.*s\"", node->str_size2, node->str_value2, node->str_size, node->str_value);
                            return 1;
                        }
                        else
                        {
                            write_compiler_error(node->filename, node->file_line, "Cyclic reference of inline \"%.*s\"", node->str_size, node->str_value);
                            return 1;
                        }
                    }

                    // inline
                    inline_node = duplicate_node_deep(inline_node);

                    if (inline_node->children_count == 0)
                    {
                        break;
                    }

                    struct ASTNode *last_node = current_node->children[1], *last_cur_node, *one_before_last = NULL;                    

                    push_inline_symbol_stack(node->str_value, node->str_size, node->str_value2, node->str_size2);
                    int inner_length = 0;
                    if (recursive_first_pass(inline_node, &inner_length)) { return 1; }
                    *length += inner_length;
                    pop_inline_symbol_stack();

                    current_node->children[0] = inline_node->children[0];
                    one_before_last = current_node;
                    last_cur_node = current_node->children[1] = inline_node->children[1];

                    while(last_cur_node->children_count != 0)
                    {
                        one_before_last = last_cur_node;
                        last_cur_node = last_cur_node->children[1];
                    }
                    one_before_last->children[1] = last_node;

                    current_node = one_before_last;
                }
                else
                {
                    // function call

                    struct ASTNode *call_node = create_node_str(NODE_TYPE_OP, NULL, "call", 4);
                    call_node->filename = node->filename;
                    call_node->file_line = node->file_line;
                    struct ASTNode *expression_node = create_node_str2(NODE_TYPE_EXPRESSION, NULL, node->str_value, node->str_size, node->str_value2, node->str_size2);
                    expression_node->filename = node->filename;
                    expression_node->file_line = node->file_line;
                    call_node->children[0] = expression_node;
                    call_node->children_count = 1;

                    current_node->children[0] = call_node;

                    int inner_length = 0;
                    compile_op(call_node, FALSE, &inner_length);
                    *length += inner_length;
                }
                break;
            }
            case NODE_TYPE_IF:
            {
                BOOL has_else = (node->children_count == 2);

                int if_clause_length = 0, else_clause_length = 0;
                if (recursive_first_pass(node->children[0], &if_clause_length)) { return 1; }
                write_debug("Size inside if in line %d is %d bytes", node->file_line, if_clause_length);
                if (has_else)
                {
                    if (recursive_first_pass(node->children[1], &else_clause_length)) { return 1; }
                    write_debug("Size inside else in line %d is %d bytes", node->file_line, else_clause_length);
                }
                *length += if_clause_length + else_clause_length;

                char *label1 = get_new_label();
                struct ASTNode *jp_node = NULL;
                char *label2 = get_new_label();
                struct ASTNode *label2_node = create_node_str(NODE_TYPE_LABEL, NULL, label2, strlen(label2));
                struct ASTNode *jp2_node = NULL;

                int inner_length = 0;

                if (has_else)
                {
                    if (else_clause_length < 128)
                    {
                        jp2_node = create_jr_node(label2);
                    }
                    else
                    {
                        jp2_node = create_jp_node(label2);
                    }

                    if (compile_op(jp2_node, FALSE, &inner_length)) { return 1; }
                    *length += inner_length;
                    if_clause_length += inner_length;
                }

                if (cpu_type == CPU_TYPE_GB)
                {
                    if (is_str_equal(node->str_value, node->str_size, "p") ||
                        is_str_equal(node->str_value, node->str_size, "m") ||
                        is_str_equal(node->str_value, node->str_size, "pe") ||
                        is_str_equal(node->str_value, node->str_size, "po"))
                    {
                        write_compiler_error(node->filename, node->file_line, "Condition \"%.*s\" in if is invalid for this cpu type", node->str_size, node->str_value);
                        return 1;
                    }
                }

                if (if_clause_length > 127)
                {
                    jp_node = create_reverse_jp_cond_node(node->str_value, node->str_size, label1);
                }
                else
                {
                    jp_node = create_reverse_jr_cond_node(node->str_value, node->str_size, label1);
                }                

                if (compile_op(jp_node, FALSE, &inner_length)) { return 1; }
                *length += inner_length;

                assert(jp_node != NULL);

                struct ASTNode *label1_node = create_node_str(NODE_TYPE_LABEL, NULL, label1, strlen(label1));

                struct ASTNode *last_node = current_node->children[1], *last_cur_node;
                
                current_node->children[0] = jp_node;

                last_cur_node = current_node->children[1] = node->children[0];                

                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                if (!has_else)
                {
                    last_cur_node->children_count = 2;
                    last_cur_node->children[0] = label1_node;
                    last_cur_node->children[1] = last_node;

                    current_node = last_cur_node;

                    break;
                }                

                last_cur_node->children_count = 2;
                last_cur_node->children[0] = jp2_node;

                struct ASTNode *new_node = create_node(NODE_TYPE_MAIN, NULL);
                last_cur_node->children[1] = new_node;

                new_node->children_count = 2;
                new_node->children[0] = label1_node;

                last_cur_node = new_node->children[1] = node->children[1];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                last_cur_node->children_count = 2;
                last_cur_node->children[0] = label2_node;
                last_cur_node->children[1] = last_node;

                current_node = last_cur_node;

                break;
            }
            case NODE_TYPE_WHILE:
            {
                char *label1 = get_new_label(), *label2 = get_new_label();

                push_loop_label(label2);

                int inner_while_length = 0, inner_length = 0;
                if (recursive_first_pass(node->children[0], &inner_while_length)) { return 1; }
                *length += inner_while_length;
                write_debug("Size inside while in line %d is %d bytes", node->file_line, inner_while_length);

                pop_loop_label();

                struct ASTNode *jp_node = NULL;
                
                if (cpu_type == CPU_TYPE_GB)
                {
                    if (is_str_equal(node->str_value, node->str_size, "p") ||
                        is_str_equal(node->str_value, node->str_size, "m") ||
                        is_str_equal(node->str_value, node->str_size, "pe") ||
                        is_str_equal(node->str_value, node->str_size, "po"))
                    {
                        write_compiler_error(node->filename, node->file_line, "Condition \"%.*s\" in while is invalid for this cpu type", node->str_size, node->str_value);
                        return 1;
                    }
                }

                if (inner_while_length < 124)
                {
                    jp_node = create_reverse_jr_cond_node(node->str_value, node->str_size, label2);
                }
                else
                {
                    jp_node = create_reverse_jp_cond_node(node->str_value, node->str_size, label2);
                }
                assert(jp_node != NULL);
                if (compile_op(jp_node, FALSE, &inner_length)) { return 1; }
                *length += inner_length;

                struct ASTNode *label1_node = create_node_str(NODE_TYPE_LABEL, NULL, label1, strlen(label1));
                struct ASTNode *new_node = create_node(NODE_TYPE_MAIN, NULL);                

                struct ASTNode *last_node = current_node->children[1], *last_cur_node;
                current_node->children[0] = label1_node;
                current_node->children[1] = new_node;
                current_node->children_count = 2;

                new_node->children_count = 2;
                new_node->children[0] = jp_node;
                last_cur_node = new_node->children[1] = node->children[0];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                struct ASTNode *label2_node = create_node_str(NODE_TYPE_LABEL, NULL, label2, strlen(label2));
                struct ASTNode *jp2_node = NULL;
                
                if (inner_while_length < 124)
                {
                    jp2_node = create_jr_node(label1);
                }
                else
                {
                    jp2_node = create_jp_node(label1);
                }
                if (compile_op(jp2_node, FALSE, &inner_length)) { return 1; }
                *length += inner_length;


                last_cur_node->children_count = 2;
                last_cur_node->children[0] = jp2_node;

                new_node = create_node(NODE_TYPE_MAIN, NULL);
                last_cur_node->children[1] = new_node;

                new_node->children_count = 2;
                new_node->children[0] = label2_node;

                last_cur_node = new_node;
                last_cur_node->children[1] = last_node;

                current_node = last_cur_node;

                break;
            }
            case NODE_TYPE_DO:
            {
                char *label1 = get_new_label(), *label2 = get_new_label();

                push_loop_label(label2);

                int inner_do_length = 0, inner_length = 0;
                if (recursive_first_pass(node->children[0], &inner_do_length)) { return 1; }
                *length += inner_do_length;
                write_debug("Size inside do in line %d is %d bytes", node->file_line, inner_do_length);

                pop_loop_label();

                if (cpu_type == CPU_TYPE_GB)
                {
                    if (is_str_equal(node->str_value, node->str_size, "p") ||
                        is_str_equal(node->str_value, node->str_size, "m") ||
                        is_str_equal(node->str_value, node->str_size, "pe") ||
                        is_str_equal(node->str_value, node->str_size, "po"))
                    {
                        write_compiler_error(node->filename, node->file_line, "Condition \"%.*s\" in do is invalid for this cpu type", node->str_size, node->str_value);
                        return 1;
                    }
                }

                struct ASTNode *jp_node = NULL;
                if (inner_do_length < 127)
                {
                    jp_node = create_jr_cond_node(node->str_value, node->str_size, label1);
                }
                else
                {
                    jp_node = create_jp_cond_node(node->str_value, node->str_size, label1);
                }
                assert(jp_node != NULL);
                if (compile_op(jp_node, FALSE, &inner_length)) { return 1; }
                *length += inner_length;

                struct ASTNode *label1_node = create_node_str(NODE_TYPE_LABEL, NULL, label1, strlen(label1));

                struct ASTNode *last_node = current_node->children[1], *last_cur_node;
                current_node->children[0] = label1_node;
                current_node->children_count = 2;
                last_cur_node = current_node->children[1] = node->children[0];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                last_cur_node->children_count = 2;
                last_cur_node->children[0] = jp_node;
                
                struct ASTNode *new_node = add_main_node_to_end(create_node_str(NODE_TYPE_LABEL, NULL, label2, strlen(label2)), last_cur_node);
                new_node->children_count = 2;
                new_node->children[1] = last_node;

                current_node = new_node;

                break;
            }
            case NODE_TYPE_FOREVER:
            {
                char *label1 = get_new_label(), *label2 = get_new_label();

                push_loop_label(label2);

                int inner_forever_length = 0, inner_length = 0;
                if (recursive_first_pass(node->children[0], &inner_forever_length)) { return 1; }
                *length += inner_forever_length;
                write_debug("Size inside forever in line %d is %d bytes", node->file_line, inner_forever_length);

                pop_loop_label();

                struct ASTNode *jp_node = NULL;                
                if (inner_forever_length < 127)
                {
                    jp_node = create_jr_node(label1);
                }
                else
                {
                    jp_node = create_jp_node(label1);
                }
                assert(jp_node != NULL);
                if (compile_op(jp_node, FALSE, &inner_length)) { return 1; }
                *length += inner_length;

                struct ASTNode *label1_node = create_node_str(NODE_TYPE_LABEL, NULL, label1, strlen(label1));

                struct ASTNode *last_node = current_node->children[1], *last_cur_node;
                current_node->children[0] = label1_node;
                current_node->children_count = 2;
                last_cur_node = current_node->children[1] = node->children[0];
                while(last_cur_node->children_count != 0)
                {
                    last_cur_node = last_cur_node->children[1];
                }

                last_cur_node->children_count = 2;
                last_cur_node->children[0] = jp_node;

                struct ASTNode *new_node = add_main_node_to_end(create_node_str(NODE_TYPE_LABEL, NULL, label2, strlen(label2)), last_cur_node);
                new_node->children_count = 2;
                new_node->children[1] = last_node;

                current_node = new_node;

                break;
            }
            case NODE_TYPE_BREAK:
            {
                char *label = peek_loop_label();
                if (label == NULL)
                {
                    write_compiler_error(node->filename, node->file_line, "break not inside of loop", 0);
                    return 1;
                }

                struct ASTNode *last_node = current_node->children[1];

                struct ASTNode *new_node = add_main_node_to_end(create_jp_node(label), current_node);
                new_node->children_count = 2;
                new_node->children[1] = last_node;

                break;
            }
            case NODE_TYPE_SET_CPU_TYPE:
            {
                cpu_type = (enum CPUType)node->num_value;
                break;
            }
            default:
                break;
        }

        current_node = current_node->children[1];
    }

    return 0;
}   

static int compile_struct_init(struct ASTNode *struct_init, struct StructuredType *structured_type, int offset)
{
    struct ASTNode *struct_init_element = struct_init->children[0];
    while (struct_init_element != NULL)
    {
        struct StructElement *struct_element = get_struct_element(structured_type, struct_init_element->str_value, struct_init_element->str_size);
        struct StructuredType *inner_structured_type = NULL;

        if (struct_element == NULL)
        {
            write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Element \"%.*s\" not found in structure", struct_init_element->str_size, struct_init_element->str_value);
            return 1;
        }

        BOOL is_struct_element_native = is_native_type(struct_element->type, struct_element->type_size);

        if (!is_struct_element_native)
        {
            inner_structured_type = get_structured_type(struct_element->type, struct_element->type_size, struct_element->type_library, struct_element->type_library_size);
            if (inner_structured_type == NULL)
            {
                if (struct_element->type_library_size > 0)
                {
                    write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Type \"%.*s::%.*s\" not found", struct_element->type_library_size, struct_element->type_library, struct_element->type_size, struct_element->type);
                }
                else
                {
                    write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Type \"%.*s\" not found", struct_element->type_size, struct_element->type);
                }
            }
        }

        if (struct_element == NULL)
        {
            write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Structure element \"%.*s\" not found", struct_init_element->str_size, struct_init_element->str_value);
            return 1;
        }

        write_debug("Setting value for element \"%.*s\"", struct_init_element->str_size, struct_init_element->str_value);

        int index = 0;
        struct ASTNode *struct_init_element_value = struct_init_element->children[0];        
        while (struct_init_element_value != NULL)
        {
            if (index + 1 > struct_element->array_length)
            {
                write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Initialization of values beyond the length of \"%.*s\" which is %d", struct_element->name_size, struct_element->name, struct_element->array_length);
                return 1;
            }

            if (is_struct_element_native)
            {
                if (struct_init_element_value->children[0]->type != NODE_TYPE_EXPRESSION)
                {
                    write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Expected expression for element \"%.*s\"", struct_element->name_size, struct_element->name);
                    return 1;
                }

                set_struct_bytes(structured_type, offset, struct_element, index, struct_init_element_value->children[0]);
            }
            else
            {
                if (struct_init_element_value->children[0]->type != NODE_TYPE_STRUCT_INIT)
                {
                    write_compiler_error(struct_init_element->filename, struct_init_element->file_line, "Expected structure initializer for element \"%.*s\"", struct_element->name_size, struct_element->name);
                    return 1;
                }

                if (compile_struct_init(struct_init_element_value->children[0], inner_structured_type, struct_element->position + (index * inner_structured_type->struct_size))) { return 1; }
            }

            struct_init_element_value = struct_init_element_value->children[1];
            index++;
        }

        struct_init_element = struct_init_element->children[1];
    }
    return 0;
}

int compiler_current_address = 0;

static void fprint_start_list(FILE *fp, struct ASTNode *node)
{
    fprintf(fp, "%04X\t", compiler_current_address);
    if (node->file_line > 0)
    {
        fprintf(fp, "%35s:%-5d\t", node->filename, node->file_line);
    }
    else
    {
        fprintf(fp, "%35s %5s\t", "  ", " ");
    }
}

static void fprint_identifier(FILE *fp, struct ASTNode *node)
{
    if (node->str_size2 > 0)
    {
        fprintf(fp_list, "%.*s::%.*s", node->str_size2, node->str_value2, node->str_size, node->str_value);
    }
    else
    {
        fprintf(fp_list, "%.*s", node->str_size, node->str_value);
    }
}

static int fprint_db_count = 0;
enum FprintDbType
{
    FPRINT_DB_TYPE_BYTE,
    FPRINT_DB_TYPE_WORD,
    FPRINT_DB_TYPE_DWORD
};

static void fprint_db_list(FILE *fp, struct ASTNode *node, uint32_t value, struct ASTNode *expression_node, enum FprintDbType type)
{
    if (fprint_db_count % 16 == 0)
    {
        if (fprint_db_count > 0)
        {
            fprintf(fp, "\n");
        }
        fprint_start_list(fp, node);
        switch(type)
        {
            case FPRINT_DB_TYPE_BYTE:
                fprintf(fp, "\tBYTE ");
                break;
            case FPRINT_DB_TYPE_WORD:
                fprintf(fp, "\tWORD ");
                break;
            case FPRINT_DB_TYPE_DWORD:
                fprintf(fp, "\tDWORD ");
                break;
        }
    }
    else
    {
        fprintf(fp, ", ");
    }

    if (expression_node == NULL)
    {
        fprintf(fp, "%"PRIu32"", value);
    }
    else
    {
        fprint_operand_node(fp, expression_node);
    }

    fprint_db_count++;
}

static void fprint_db_list_end(FILE *fp)
{
    fprintf(fp, "\n");
    fprint_db_count = 0;
}

static void fprint_data_list(FILE *fp, struct ASTNode *node, int size, int length)
{
    fprint_start_list(fp, node);
    fprintf(fp, "; sizeof(");
    fprint_identifier(fp, node);
    fprintf(fp, ") = %d\n", size);
    fprint_start_list(fp, node);
    fprintf(fp, "; length(");
    fprint_identifier(fp, node);
    fprintf(fp, ") = %d\n", length);
}

static int second_pass(struct ASTNode *first_node)
{
    struct ASTNode *current_node = first_node, *node;

    write_debug("Compiler second pass...", 0);

    compiler_current_address = 0;

    while (current_node != NULL && current_node->children_count > 0)
    {
        node = current_node->children[0];

        assert(current_node->type == NODE_TYPE_MAIN);

        switch(node->type)
        {
            case NODE_TYPE_OP:
            {
                int old_output_length = output_length;
                int op_length = -1;
                if (compile_op(node, TRUE, &op_length))
                {
                    return 1;
                }

                if (fp_list != NULL)
                {
                    fprint_start_list(fp_list, node);
                    fprintf(fp_list, "\t");
                    fprint_op(fp_list, node);
                }

                assert(op_length > 0 && op_length < MAX(5, MAX_AST_NODE_CHILDREN+1));
                last_output_elem->set_address = TRUE;
                compiler_current_address += output_length - old_output_length;
                break;
            }
            case NODE_TYPE_LABEL:
            {
                if (is_constant_present(node->str_value2, node->str_size2, node->str_value, node->str_size))
                {
                    write_compiler_error(node->filename, node->file_line, "Symbol \"%.*s\" already defined", node->str_size, node->str_value);
                    return 1;
                }
                set_constant(node->str_value2, node->str_size2, node->str_value, node->str_size, compiler_current_address);

                if (fp_list != NULL)
                {
                    fprint_start_list(fp_list, node);
                    fprint_identifier(fp_list, node);
                    fprintf(fp_list, ":\n");
                }

                break;
            }
            case NODE_TYPE_DATA:
            {
                if (node->str_size > 0)
                {
                    // check if this data is inside a library and needed
                    if (node->str_size2 > 0 && !is_library_symbol_needed(node->str_value2, node->str_size2, node->str_value, node->str_size))
                    {
                        write_debug("Data \"%.*s::%.*s\" unneeded", node->str_size2, node->str_value2, node->str_size, node->str_value);
                        break;
                    }

                    if (is_constant_present(node->str_value2, node->str_size2, node->str_value, node->str_size))
                    {
                        write_compiler_error(node->filename, node->file_line, "Symbol \"%.*s\" already defined", node->str_size, node->str_value);
                        return 1;
                    }
                    set_constant(node->str_value2, node->str_size2, node->str_value, node->str_size, compiler_current_address);

                    if (fp_list != NULL)
                    {
                        fprint_start_list(fp_list, node);
                        fprint_identifier(fp_list, node);
                        fprintf(fp_list, ":\n");
                    }
                }
                
                struct StructuredType *structured_type = NULL;
                if (!is_native_type(node->children[0]->str_value, node->children[0]->str_size))
                {
                    structured_type = get_structured_type(
                        node->children[0]->str_value, node->children[0]->str_size,
                        node->children[0]->str_value2, node->children[0]->str_size2);                
                    if (structured_type == NULL)
                    {
                        if (node->children[0]->str_size2 > 0)
                        {
                            write_compiler_error(node->filename, node->file_line, "Type \"%.*s::%.*s\" not found", node->children[0]->str_size2, node->children[0]->str_value2, node->children[0]->str_size, node->children[0]->str_value);
                        }
                        else
                        {
                            write_compiler_error(node->filename, node->file_line, "Type \"%.*s\" not found", node->children[0]->str_size, node->children[0]->str_value);
                        }
                        return 1;
                    }
                }                

                if (node->children[1]->type == NODE_TYPE_DATA_VALUE)
                {
                    int data_length = 0;
                    struct ASTNode *node_value = node->children[1];
                    while (node_value != NULL)
                    {                    
                        if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "byte"))
                        {
                            if (node_value->children[0]->type == NODE_TYPE_EXPRESSION)
                            {
                                node_value->children[0]->type = NODE_TYPE_EXPRESSION_8;
                                add_output_element_set_address(0, node_value->children[0]);
                                compiler_current_address++;

                                if (fp_list != NULL) fprint_db_list(fp_list, node, 0, node_value->children[0], FPRINT_DB_TYPE_BYTE);
                            }
                            else if (node_value->children[0]->type == NODE_TYPE_STRING)
                            {
                                for(int i = 0; i < node_value->children[0]->str_size; i++)
                                {
                                    add_output_element_set_address(node_value->children[0]->str_value[i], NULL);

                                    if (fp_list != NULL) fprint_db_list(fp_list, node, node_value->children[0]->str_value[i], NULL, FPRINT_DB_TYPE_BYTE);
                                }
                                compiler_current_address += node_value->children[0]->str_size;
                                data_length += node_value->children[0]->str_size - 1;
                            }
                            else
                            {
                                write_compiler_error(node->filename, node->file_line, "Expression or string literal expected in data initializer", 0);
                                return 1;
                            }
                        }
                        else if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "word"))
                        {
                            if (node_value->children[0]->type != NODE_TYPE_EXPRESSION)
                            {
                                write_compiler_error(node->filename, node->file_line, "Expression expected in data initializer", 0);
                                return 1;
                            }

                            node_value->children[0]->type = NODE_TYPE_EXPRESSION_16;
                            add_output_element(0, node_value->children[0]);
                            add_output_element_set_address(0, NULL);
                            compiler_current_address += 2;

                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, node_value->children[0], FPRINT_DB_TYPE_WORD);
                        }
                        else if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "dword"))
                        {
                            if (node_value->children[0]->type != NODE_TYPE_EXPRESSION)
                            {
                                write_compiler_error(node->filename, node->file_line, "Expression expected in data initializer", 0);
                                return 1;
                            }

                            node_value->children[0]->type = NODE_TYPE_EXPRESSION_32;
                            add_output_element(0, node_value->children[0]);
                            add_output_element(0, NULL);
                            add_output_element(0, NULL);
                            add_output_element_set_address(0, NULL);
                            compiler_current_address += 4;

                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, node_value->children[0], FPRINT_DB_TYPE_DWORD);
                        }
                        else
                        {
                            if (node_value->children[0]->type != NODE_TYPE_STRUCT_INIT)
                            {
                                write_compiler_error(node->filename, node->file_line, "Structure initialization expected", 0);
                                return 1;
                            }

                            struct ASTNode *struct_init = node_value->children[0];

                            clear_struct_bytes(structured_type->struct_size);
                            if (compile_struct_init(struct_init, structured_type, 0)) { return 1; }
                            int list_skip_bytes = 0;
                            for(int i = 0; i < structured_type->struct_size; i++)
                            {
                                add_output_element_set_address(0, struct_bytes[i]);                    

                                if (i > 0)
                                {
                                    if (fp_list != NULL) fprint_db_list_end(fp_list);
                                }

                                if (struct_bytes[i] != NULL)
                                {
                                    switch(struct_bytes[i]->type)
                                    {
                                        case NODE_TYPE_EXPRESSION_16:
                                        {
                                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, struct_bytes[i], FPRINT_DB_TYPE_WORD);
                                            list_skip_bytes = 2;
                                            break;
                                        }
                                        case NODE_TYPE_EXPRESSION_32:
                                        {
                                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, struct_bytes[i], FPRINT_DB_TYPE_DWORD);
                                            list_skip_bytes = 4;
                                            break;
                                        }
                                        default:
                                        {
                                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, struct_bytes[i], FPRINT_DB_TYPE_BYTE);
                                            list_skip_bytes = 1;
                                        }
                                    }
                                }
                                else if (list_skip_bytes == 0)
                                {
                                    if (fp_list != NULL) fprint_db_list(fp_list, node, 0, struct_bytes[i], FPRINT_DB_TYPE_BYTE);
                                }

                                if (list_skip_bytes > 0)
                                {
                                    list_skip_bytes--;
                                }
                            }
                            compiler_current_address += structured_type->struct_size;
                        }

                        data_length++;
                        node_value = node_value->children[1];
                    }

                    if (fp_list != NULL) fprint_db_list_end(fp_list);

                    if (node->str_size > 0)
                    {
                        add_data_symbol(node->str_value, node->str_size,
                            node->str_value2, node->str_size2,
                            node->children[0]->str_value, node->children[0]->str_size,
                            node->children[0]->str_value2, node->children[0]->str_size2, data_length);

                        if (fp_list != NULL) fprint_data_list(fp_list, node, get_last_data_symbol()->size_of_type * data_length, data_length);
                    }                    
                }
                else if (node->children[1]->type == NODE_TYPE_DATA_SIZE)
                {
                    struct ASTNode *expression_node = node->children[1]->children[0];
                    expression_node->type = NODE_TYPE_EXPRESSION_16;
                    int64_t value;
                    if (resolve_expression(expression_node, &value)) return 1;

                    if (value < 1)
                    {
                        write_compiler_error(node->filename, node->file_line, "Data length can't be less than 1", 0);
                        return 1;
                    }
                    
                    if (node->str_size > 0)
                    {
                        add_data_symbol(node->str_value, node->str_size,
                            node->str_value2, node->str_size2,
                            node->children[0]->str_value, node->children[0]->str_size,
                            node->children[0]->str_value2, node->children[0]->str_size2, (int)value);
                    }

                    // TODO: check value versus address positions left

                    for(int i = 0; i < value; i++)
                    {
                        if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "byte"))
                        {                            
                            add_output_element_set_address(0, NULL);
                            compiler_current_address++;

                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, NULL, FPRINT_DB_TYPE_BYTE);
                        }
                        else if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "word"))
                        {
                            add_output_element(0, NULL);
                            add_output_element_set_address(0, NULL);
                            compiler_current_address += 2;

                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, NULL, FPRINT_DB_TYPE_WORD);
                        }
                        else if (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "dword"))
                        {
                            add_output_element(0, NULL);
                            add_output_element(0, NULL);
                            add_output_element(0, NULL);
                            add_output_element_set_address(0, NULL);
                            compiler_current_address += 4;

                            if (fp_list != NULL) fprint_db_list(fp_list, node, 0, NULL, FPRINT_DB_TYPE_DWORD);
                        }
                        else
                        {
                            for(int j = 0; j < structured_type->struct_size; j++)
                            {
                                add_output_element_set_address(0, NULL);
                                if (fp_list != NULL) fprint_db_list(fp_list, node, 0, NULL, FPRINT_DB_TYPE_BYTE);
                            }
                            compiler_current_address += structured_type->struct_size;
                        }                        
                    }

                    if (fp_list != NULL) fprint_db_list_end(fp_list);

                    if (node->str_size > 0)
                    {
                        if (fp_list != NULL) fprint_data_list(fp_list, node, get_last_data_symbol()->size_of_type * get_last_data_symbol()->length, get_last_data_symbol()->length);
                    }
                }
                else if (node->children[1]->type == NODE_TYPE_DATA_FROM)
                {
                    struct ASTNode *from_node = node->children[1];

                    char *new_filename = (char*)malloc(256);
                    if (!get_file_include_path(new_filename, from_node->str_value, from_node->filename))
                    {
                        write_compiler_error(from_node->filename, from_node->file_line, "Error including binary file \"%s\"", from_node->str_value);
                        return 1;
                    }
                    
                    FILE *fp = fopen(new_filename, "rb");
                    if (fp == NULL)
                    {
                        write_compiler_error(node->filename, node->file_line, "Unable to open file \"%s\" in data statement", new_filename);
                        return 1;
                    }

                    fseek(fp, 0, SEEK_END);
                    long file_size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);

                    int size_of_type = 0;
                    if (structured_type != NULL)
                    {
                        size_of_type = structured_type->struct_size;
                    }
                    else
                    {
                        size_of_type = get_native_type_size(node->children[0]->str_value, node->children[0]->str_size);
                    }

                    if (file_size % size_of_type != 0)
                    {
                        if (node->children[0]->str_size2 > 0)
                        {
                            write_compiler_error(node->filename, node->file_line, "Size of file \"%s\" is not a multiple of the size of data type \"%.*s::%.*s\".", new_filename, node->children[0]->str_size2, node->children[0]->str_value2, node->children[0]->str_size, node->children[0]->str_value);
                        }
                        else
                        {
                            write_compiler_error(node->filename, node->file_line, "Size of file \"%s\" is not a multiple of the size of data type \"%.*s\".", new_filename, node->children[0]->str_size, node->children[0]->str_value);
                        }
                        return 1;
                    }

                    if (node->str_size > 0)
                    {
                        add_data_symbol(node->str_value, node->str_size,
                            node->str_value2, node->str_size2,
                            node->children[0]->str_value, node->children[0]->str_size,
                            node->children[0]->str_value2, node->children[0]->str_size2, file_size / size_of_type);
                    }

                    uint8_t byte_read;
                    int bytes_read_count = fread(&byte_read, 1, 1, fp);
                    while (bytes_read_count)
                    {
                        add_output_element(byte_read, NULL);
                        bytes_read_count = fread(&byte_read, 1, 1, fp);

                        if (fp_list != NULL) fprint_db_list(fp_list, node, byte_read, NULL, FPRINT_DB_TYPE_BYTE);
                    }

                    compiler_current_address += file_size;

                    if (fp_list != NULL) fprint_db_list_end(fp_list);

                    if (node->str_size > 0)
                    {
                        if (fp_list != NULL) fprint_data_list(fp_list, node, get_last_data_symbol()->size_of_type * get_last_data_symbol()->length, get_last_data_symbol()->length);
                    }
                }
                else
                {
                    assert(1==0 && "Invalid Data Node");
                }
                
                break;
            }
            case NODE_TYPE_CONST:
            {
                int64_t value;

                if (resolve_expression(node->children[0], &value)) { return 1; }

                if (is_constant_present(node->str_value2, node->str_size2, node->str_value, node->str_size))
                {
                    write_compiler_error(node->filename, node->file_line, "Symbol \"%.*s\" already defined", node->str_size, node->str_value);
                    return 1;
                }

                set_constant(node->str_value2, node->str_size2, node->str_value, node->str_size, value);
                break;
            }
            case NODE_TYPE_STRUCT:
            case NODE_TYPE_UNION:
            {
                char *struct_name;
                int struct_name_size;
                if (build_structured_type(node, &struct_name, &struct_name_size)) { return 1; }
                break;
            }
            case NODE_TYPE_ORIGIN:
            {
                add_output_element_not_final(node);
                node->children[0]->type = NODE_TYPE_EXPRESSION_16;
                struct ASTNode *origin_expression_node = node->children[0];
                int64_t result = 0;
                if (resolve_expression(origin_expression_node, &result))
                {
                    write_compiler_error(node->filename, node->file_line, "Expression in #origin cannot be resolved", 0);
                    return 1;
                }
                compiler_current_address = (int)result;

                break;
            }
            case NODE_TYPE_PRINT:
            case NODE_TYPE_OUTPUT_ON:
            case NODE_TYPE_OUTPUT_OFF:
            case NODE_TYPE_SET_OUTPUT_FILE:
            {
                add_output_element_not_final(node);
                break;
            }
            case NODE_TYPE_INCLUDE_BINARY:
            {
                char *new_filename = (char*)malloc(256);
                if (!get_file_include_path(new_filename, node->str_value, node->filename))
                {
                    write_compiler_error(node->filename, node->file_line, "Error including binary file \"%s\"", node->str_value);
                    return 1;
                }

                FILE *fp = fopen(new_filename, "rb");
                if (fp == NULL)
                {
                    write_compiler_error(node->filename, node->file_line, "Unable to open binary file \"%s\"", new_filename);
                    return 1;
                }

                uint8_t byte_read;
                int bytes_read_count = fread(&byte_read, 1, 1, fp);
                while (bytes_read_count)
                {
                    add_output_element(byte_read, NULL);
                    bytes_read_count = fread(&byte_read, 1, 1, fp);
                }

                break;
            }
            case NODE_TYPE_SET_CPU_TYPE:
            {
                cpu_type = (enum CPUType)node->num_value;
                break;
            }
            default:
                break;
        }
        current_node = current_node->children[1];        
    }

    return 0;
}

char *compiler_output_filename;
static FILE *compiler_fp_output = NULL;

static void write_output_byte(uint8_t value)
{
    if (!compiler_fp_output)
    {
        compiler_fp_output = fopen(compiler_output_filename, "wb");
    }

    fwrite(&(value), 1, 1, compiler_fp_output);
}

static int third_pass(struct ASTNode *node)
{
    int64_t value;
    struct OutputElement *current_output_elem = first_output_elem;
    BOOL write_output_content = TRUE;

    write_debug("Compiler third pass...", 0);

    int current_address = compiler_current_address = 0;

    while(current_output_elem != NULL)
    {        
        if (current_output_elem->node != NULL)
        {
            switch(current_output_elem->node->type)
            {
                case NODE_TYPE_EXPRESSION:
                case NODE_TYPE_EXPRESSION_8:
                {
                    if (resolve_expression(current_output_elem->node, &value)) return 1; 
                    current_output_elem->value = value;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    break;
                }
                case NODE_TYPE_EXPRESSION_8c:
                case NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS:
                {
                    if (resolve_expression(current_output_elem->node, &value)) return 1; 
                    current_output_elem->value = (value-2);
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    break;
                }
                case NODE_TYPE_EXPRESSION_16:
                {
                    if (resolve_expression(current_output_elem->node, &value)) return 1;
                    current_output_elem->value = value & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_output_elem = current_output_elem->next;
                    current_output_elem->value = (value >> 8) & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_address++;
                    break;                    
                }
                case NODE_TYPE_EXPRESSION_32:
                {
                    if (resolve_expression(current_output_elem->node, &value)) return 1;
                    current_output_elem->value = value & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_output_elem = current_output_elem->next;
                    current_output_elem->value = (value >> 8) & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_output_elem = current_output_elem->next;
                    current_output_elem->value = (value >> 8) & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_output_elem = current_output_elem->next;
                    current_output_elem->value = (value >> 8) & 0xFF;
                    current_output_elem->node = NULL;
                    if (write_output_content) write_output_byte(current_output_elem->value);
                    current_address+=3;
                    break;                    
                }
                case NODE_TYPE_MATCH_LIST:
                {
                    assert(current_output_elem->node->children_count == 2);
                    if (resolve_expression(current_output_elem->node->children[1], &value)) return 1;
                    struct ASTNode *current_node = current_output_elem->node;
                    do {
                        if (value == current_node->num_value)
                        {
                            current_output_elem->value = current_node->num_value2;
                            current_output_elem->node = NULL;

                            if (write_output_content) write_output_byte(current_output_elem->value);
                            break;
                        }
                        current_node = current_node->children[0];
                    } while(current_node != NULL);
                    if (current_output_elem->node)
                    {
                        write_compiler_error(current_output_elem->node->children[1]->filename, current_output_elem->node->children[1]->file_line, "Invalid value %"PRId64"", value);
                        return 1;
                    }
                    break;
                }
                case NODE_TYPE_PRINT:
                {
                    struct ASTNode *data_node = current_output_elem->node->children[0];
                    int64_t expression_result = 0;
                    write_print_start(current_output_elem->node->filename, current_output_elem->node->file_line);
                    do
                    {
                        if (data_node->children[0] == NULL)
                        {
                            write_print_string(data_node->str_value, data_node->str_size);
                        }
                        else
                        {
                            if (resolve_expression(data_node->children[0], &expression_result))
                            {
                                write_compiler_error(data_node->filename, data_node->file_line, "Error in print statement", 0);
                                return 1;
                            }
                            write_print_expression(expression_result);
                        }

                        data_node = data_node->children[1];
                    } while (data_node != NULL);
                    write_print_end();
                                        
                    break;
                }
                case NODE_TYPE_ORIGIN:
                {
                    struct ASTNode *origin_expression_node = current_output_elem->node->children[0];
                    int64_t result = 0;
                    if (resolve_expression(origin_expression_node, &result)) { return 1; }
                    compiler_current_address = current_address = (int)result;

                    break;
                }
                case NODE_TYPE_OUTPUT_ON:
                {
                    write_output_content = TRUE;
                    break;
                }
                case NODE_TYPE_OUTPUT_OFF:
                {
                    write_output_content = FALSE;
                    break;
                }
                case NODE_TYPE_SET_OUTPUT_FILE:
                {
                    write_debug("Changing output file to \"%s\"", current_output_elem->node->str_value);
                    if (compiler_fp_output)
                    {
                        fclose(compiler_fp_output);
                        compiler_fp_output = NULL;
                    }
                    compiler_output_filename = current_output_elem->node->str_value;
                    break;
                }
                default:
                {
                    assert(1 == 0 && "Unexpected AST node type");
                }
            }
        }
        else if (current_output_elem->final)
        {
            if (write_output_content) write_output_byte(current_output_elem->value);
        }

        if (current_output_elem != NULL && current_output_elem->final)
        {
            current_address++;
            if (current_output_elem->set_address)
            {
                compiler_current_address = current_address;
            }
        }
        current_output_elem = current_output_elem->next;
    }

    return 0;
}

int compile(struct ASTNode *first_node)
{
    write_debug("Compiler start", 0);

    printf("pass 1...\n");

    int length = 0;
    cpu_type = initial_cpu_type;
    if (recursive_first_pass(first_node, &length))
    {
        return 1;
    }

    #if DEBUG == 1
    FILE *fp_AST = fopen("output_ast_after_first_pass.json", "w");
    fprint_ast(fp_AST, first_node);
    fclose(fp_AST);
    #endif

    printf("pass 2...\n");

    cpu_type = initial_cpu_type;
    if (second_pass(first_node))
    {
        return 1;
    }

    printf("pass 3...\n");

    int res = 0;
    res = third_pass(first_node);
    fclose(compiler_fp_output);
    if (res)
    {
        if (remove(compiler_output_filename))
        {
            write_error("Unable to remove file \"%s\"", compiler_output_filename);
        }
        return 1;
    }

    write_debug("Compiler end", 0);

    printf("%d bytes generated\n", output_length - 1);

    return 0;
}

void fprint_output(FILE *fp)
{
    struct OutputElement *elem = first_output_elem;

    fprintf(fp, "[");

    for(int i = 0; i < output_length; i++)
    {
        if (i > 0)
        {
            fprintf(fp, ",");
        }
        if (elem->node == NULL)
        {
            fprintf(fp, "%d", elem->value);
        }
        else
        {
            fprint_ast(fp, elem->node);
        }
        elem = elem->next;
    }
    fprintf(fp, "]");
}
