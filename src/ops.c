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

static int get_op_operand_count(struct ASTNode *node)
{
    return node->children_count;
}

static BOOL is_op_name(struct ASTNode *node, char *op_name)
{
    return is_str_equal(node->str_value, node->str_size, op_name);
}

static BOOL is_op_operand1_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, register_name);
}

static BOOL is_op_operand2_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, register_name);
}

// Not used
// static BOOL is_op_operand1_register_type(struct ASTNode *node)
// {
//     return node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER;
// }

static BOOL is_op_operand2_register_type(struct ASTNode *node)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER;
}

static BOOL is_op_operand1_r_value(struct ASTNode *node)
{
    return node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER &&
        node->children[0]->str_size == 1 &&
        (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "a") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "b") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "c") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "d") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "e") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "h") ||
        is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "l"));
}

static BOOL is_op_operand2_r_value(struct ASTNode *node)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER &&
        node->children[1]->str_size == 1 &&
        (is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "a") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "b") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "c") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "d") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "e") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "h") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "l"));
}

static int get_op_operand1_r_value(struct ASTNode *node)
{
    if (node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER && node->children[0]->str_size == 1)
    {
        switch (node->children[0]->str_value[0])
        {
            case 'a': return 7;
            case 'b': return 0;
            case 'c': return 1;
            case 'd': return 2;
            case 'e': return 3;
            case 'h': return 4;
            case 'l': return 5;
        }
    }
    return -1;
}

static int get_op_operand2_r_value(struct ASTNode *node)
{
    if (node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER && node->children[1]->str_size == 1)
    {
        switch (node->children[1]->str_value[0])
        {
            case 'a': return 7;
            case 'b': return 0;
            case 'c': return 1;
            case 'd': return 2;
            case 'e': return 3;
            case 'h': return 4;
            case 'l': return 5;
        }
    }
    return -1;
}

// Not used
// static BOOL is_op_operand1_p_value(struct ASTNode *node)
// {
//     return node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER &&
//         (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "a") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "b") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "c") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "d") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "e") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "ixl") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "ixh"));
// }

static BOOL is_op_operand2_p_value(struct ASTNode *node)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER &&
        (is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "a") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "b") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "c") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "d") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "e") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "ixl") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "ixh"));
}

// Not used
// static int get_op_operand1_p_value(struct ASTNode *node)
// {
//     if (node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER)
//     {
//         switch (node->children[0]->str_value[0])
//         {
//             case 'a': return 7;
//             case 'b': return 0;
//             case 'c': return 1;
//             case 'd': return 2;
//             case 'e': return 3;
//             case 'i':
//             {
//                 if (node->children[0]->str_value[1] == 'x')
//                 {
//                     if (node->children[0]->str_value[2] == 'h') { return 4; }
//                     if (node->children[0]->str_value[2] == 'l') { return 5; }
//                 }
//             }
//         }
//     }
//     return -1;
// }

static int get_op_operand2_p_value(struct ASTNode *node)
{
    if (node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER)
    {
        switch (node->children[1]->str_value[0])
        {
            case 'a': return 7;
            case 'b': return 0;
            case 'c': return 1;
            case 'd': return 2;
            case 'e': return 3;
            case 'i':
            {
                if (node->children[1]->str_value[1] == 'x')
                {
                    if (node->children[1]->str_value[2] == 'h') { return 4; }
                    if (node->children[1]->str_value[2] == 'l') { return 5; }
                }
            }
        }
    }
    return -1;
}

// Not used
// static BOOL is_op_operand1_q_value(struct ASTNode *node)
// {
//     return node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER &&
//         (is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "a") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "b") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "c") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "d") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "e") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "iyl") ||
//         is_str_equal(node->children[0]->str_value, node->children[0]->str_size, "iyh"));
// }

static BOOL is_op_operand2_q_value(struct ASTNode *node)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER &&
        (is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "a") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "b") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "c") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "d") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "e") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "iyl") ||
        is_str_equal(node->children[1]->str_value, node->children[1]->str_size, "iyh"));
}

// Not used
// static int get_op_operand1_q_value(struct ASTNode *node)
// {
//     if (node->children_count > 0 && node->children[0]->type == NODE_TYPE_REGISTER)
//     {
//         switch (node->children[0]->str_value[0])
//         {
//             case 'a': return 7;
//             case 'b': return 0;
//             case 'c': return 1;
//             case 'd': return 2;
//             case 'e': return 3;
//             case 'i':
//             {
//                 if (node->children[0]->str_value[1] == 'y')
//                 {
//                     if (node->children[0]->str_value[2] == 'h') { return 4; }
//                     if (node->children[0]->str_value[2] == 'l') { return 5; }
//                 }
//             }
//         }
//     }
//     return -1;
// }

static int get_op_operand2_q_value(struct ASTNode *node)
{
    if (node->children_count > 1 && node->children[1]->type == NODE_TYPE_REGISTER)
    {
        switch (node->children[1]->str_value[0])
        {
            case 'a': return 7;
            case 'b': return 0;
            case 'c': return 1;
            case 'd': return 2;
            case 'e': return 3;
            case 'i':
            {
                if (node->children[1]->str_value[1] == 'y')
                {
                    if (node->children[1]->str_value[2] == 'h') { return 4; }
                    if (node->children[1]->str_value[2] == 'l') { return 5; }
                }
            }
        }
    }
    return -1;
}

static BOOL is_op_operand1_cond(struct ASTNode *node, char *cond_name)
{
    return node->children_count > 0 && 
    (
        (node->children[0]->type == NODE_TYPE_COND && is_str_equal(node->children[0]->str_value, node->children[0]->str_size, cond_name)) ||
        (node->children[0]->type == NODE_TYPE_REGISTER && is_str_equal(node->children[0]->str_value, node->children[0]->str_size, cond_name))
    );
}

// Not used
// static BOOL is_op_operand2_cond(struct ASTNode *node, char *cond_name)
// {
//     return node->children_count > 1 && 
//     (
//         (node->children[1]->type == NODE_TYPE_COND && is_str_equal(node->children[1]->str_value, node->children[1]->str_size, cond_name)) ||
//         (node->children[1]->type == NODE_TYPE_REGISTER && is_str_equal(node->children[1]->str_value, node->children[1]->str_size, cond_name))
//     );
// }

static BOOL is_op_operand1_paren_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 0 && node->children[0]->type == NODE_TYPE_LPAREN &&
        node->children[0]->children_count == 1 && node->children[0]->children[0]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[0]->children[0]->str_value, node->children[0]->children[0]->str_size, register_name);
}

static BOOL is_op_operand2_paren_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_LPAREN &&
        node->children[1]->children_count == 1 && node->children[1]->children[0]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[1]->children[0]->str_value, node->children[1]->children[0]->str_size, register_name);
}

static BOOL is_op_operand1_paren_number(struct ASTNode *node)
{
    return node->children_count > 0 && node->children[0]->type == NODE_TYPE_LPAREN &&
        node->children[0]->children_count == 1 && is_node_expression(node->children[0]->children[0]);
}

static BOOL is_op_operand2_paren_number(struct ASTNode *node)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_LPAREN &&
        node->children[1]->children_count == 1 && is_node_expression(node->children[1]->children[0]);
}

static struct ASTNode *get_op_operand1_paren_number(struct ASTNode *node)
{
    if (node->children_count > 0 && node->children[0]->type == NODE_TYPE_LPAREN &&
        node->children[0]->children_count == 1 && is_node_expression(node->children[0]->children[0]))
    {
        return node->children[0]->children[0];
    }
    return NULL;
}

static struct ASTNode *get_op_operand2_paren_number(struct ASTNode *node)
{
    if (node->children_count > 1 && node->children[1]->type == NODE_TYPE_LPAREN &&
        node->children[1]->children_count == 1 && is_node_expression(node->children[1]->children[0]))
    {
        return node->children[1]->children[0];
    }
    return NULL;
}

static BOOL is_op_operand1_paren_index_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 0 && node->children[0]->type == NODE_TYPE_LPAREN &&
        node->children[0]->children_count == 1 && node->children[0]->children[0]->type == NODE_TYPE_INDEX_REGISTER && node->children[0]->children[0]->children_count == 2 &&
        node->children[0]->children[0]->children[0]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[0]->children[0]->children[0]->str_value, node->children[0]->children[0]->children[0]->str_size, register_name);
}

static BOOL is_op_operand2_paren_index_register(struct ASTNode *node, char *register_name)
{
    return node->children_count > 1 && node->children[1]->type == NODE_TYPE_LPAREN &&
        node->children[1]->children_count == 1 && node->children[1]->children[0]->type == NODE_TYPE_INDEX_REGISTER && node->children[1]->children[0]->children_count == 2 &&
        node->children[1]->children[0]->children[0]->type == NODE_TYPE_REGISTER &&
        is_str_equal(node->children[1]->children[0]->children[0]->str_value, node->children[1]->children[0]->children[0]->str_size, register_name);
}

static BOOL is_op_operand1_number(struct ASTNode *node)
{
    return node->children_count > 0 && is_node_expression(node->children[0]);
}

static BOOL is_op_operand2_number(struct ASTNode *node)
{
    return node->children_count > 1 && is_node_expression(node->children[1]);
}

static BOOL is_op_operandn_number(struct ASTNode *node, int i)
{
    return node->children_count > i && is_node_expression(node->children[i]);
}

static struct ASTNode *get_op_operand1_number(struct ASTNode *node)
{
    if (node->children_count > 0 && is_node_expression(node->children[0]))
    {
        return node->children[0];
    }
    return NULL;
}

static struct ASTNode *get_op_operand2_number(struct ASTNode *node)
{
    if (node->children_count > 1 && is_node_expression(node->children[1]))
    {
        return node->children[1];
    }
    return NULL;
}

static struct ASTNode *get_op_operandn_number(struct ASTNode *node, int i)
{
    if (node->children_count > i && is_node_expression(node->children[i]))
    {
        return node->children[i];
    }
    return NULL;
}

static struct ASTNode *get_op_operand1_paren_index_number(struct ASTNode *node)
{
    if (node->children_count > 0 && node->children[0]->type == NODE_TYPE_LPAREN &&
        node->children[0]->children_count == 1 && node->children[0]->children[0]->type == NODE_TYPE_INDEX_REGISTER && node->children[0]->children[0]->children_count == 2)
    {
        return node->children[0]->children[0]->children[1];
    }

    return NULL;
}

static struct ASTNode *get_op_operand2_paren_index_number(struct ASTNode *node)
{
    if (node->children_count > 1 && node->children[1]->type == NODE_TYPE_LPAREN &&
        node->children[1]->children_count == 1 && node->children[1]->children[0]->type == NODE_TYPE_INDEX_REGISTER && node->children[1]->children[0]->children_count == 2)
    {
        return node->children[1]->children[0]->children[1];
    }

    return NULL;
}

int compile_op(struct ASTNode *node, BOOL add_to_output, int *length)
{
    assert(node->type == NODE_TYPE_OP);

    *length = 0;

    if (is_op_name(node, "adc"))
    {
    
        if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "hl"))
        {
            // adc a, (hl)
            if (add_to_output)
            {
                add_output_element(0x8E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_number(node))
        {
            // adc a, n
            if (add_to_output)
            {
                add_output_element(0xCE, NULL);
                node->children[1]->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, node->children[1]);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // adc a, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x8E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // adc a, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x8E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register_type(node))
        {
            // adc a, r
            if (is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xDD, NULL);
                    add_output_element(0x88 + 5, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xDD, NULL);
                    add_output_element(0x88 + 4, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xFD, NULL);
                    add_output_element(0x88 + 5, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xFD, NULL);
                    add_output_element(0x88 + 4, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_r_value(node))
            {
                if (add_to_output)
                {
                    add_output_element(0x88 + get_op_operand2_r_value(node), NULL);
                }
                *length = 1;
                return 0;
            }
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register_type(node))
        {
            // adc hl, r
            if (is_op_operand2_register(node, "bc") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x4A, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "de") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x5A, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "hl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x6A, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "sp") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x7A, NULL);
                }
                *length = 2;
                return 0;
            }
        }
    }
    else if (is_op_name(node, "add"))
    {
        if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "hl"))
        {
            // add a, (hl)
            if (add_to_output)
            {
                add_output_element(0x86, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && (is_op_operand2_paren_index_register(node, "ix")) && cpu_type != CPU_TYPE_GB)
        {
            // add a, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x86, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // add a, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x86, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_number(node))
        {
            // add a, n
            if (add_to_output)
            {
                add_output_element(0xC6, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_r_value(node))
        {
            // add a, r
            if (add_to_output)
            {
                add_output_element(0x80 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "bc"))
        {
            // add hl, bc
            if (add_to_output)
            {
                add_output_element(0x09, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "de"))
        {
            // add hl, de
            if (add_to_output)
            {
                add_output_element(0x19, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "hl"))
        {
            // add hl, hl
            if (add_to_output)
            {
                add_output_element(0x29, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "sp"))
        {
            // add hl, sp
            if (add_to_output)
            {
                add_output_element(0x39, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_register(node, "bc") && cpu_type != CPU_TYPE_GB)
        {
            // add ix, bc
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x09, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_register(node, "de") && cpu_type != CPU_TYPE_GB)
        {
            // add ix, de
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x19, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // add ix, ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x29, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_register(node, "sp") && cpu_type != CPU_TYPE_GB)
        {
            // add ix, sp
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x39, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_register(node, "bc") && cpu_type != CPU_TYPE_GB)
        {
            // add iy, bc
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x09, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_register(node, "de") && cpu_type != CPU_TYPE_GB)
        {
            // add iy, de
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x19, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // add iy, iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x29, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_register(node, "sp") && cpu_type != CPU_TYPE_GB)
        {
            // add iy, sp
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x39, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_number(node) && cpu_type == CPU_TYPE_GB)
        {
            // add sp, n
            if (add_to_output)
            {
                add_output_element(0xE8, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "and"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // and (hl)
            if (add_to_output)
            {
                add_output_element(0xA6, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // and (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xA6, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // and (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xA6, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // and n
            if (add_to_output)
            {
                add_output_element(0xE6, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // and r
            if (add_to_output)
            {
                add_output_element(0xA0 + get_op_operand1_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // and ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xA0+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // and ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xA0+4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // and iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xA0+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // and iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xA0+4, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "bit"))
    {
        if (is_op_operand1_number(node) && is_op_operand2_paren_register(node, "hl"))
        {
            // bit b, (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node(NODE_TYPE_EXPRESSION_8, NULL);            
                exp_node->str_value = "+";
                exp_node->str_size = 1;
                exp_node->children_count = 2;
                exp_node->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->str_size = 0;
                exp_node->children[0]->num_value = 0x46;
                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_value = "*";
                exp_node->children[1]->str_size = 1;
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->children[0]->str_size = 0;
                exp_node->children[1]->children[0]->num_value = 8;
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // bit b, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node(NODE_TYPE_EXPRESSION_8, NULL);            
                exp_node->str_value = "+";
                exp_node->str_size = 1;
                exp_node->children_count = 2;
                exp_node->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->str_size = 0;
                exp_node->children[0]->num_value = 0x46;
                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_value = "*";
                exp_node->children[1]->str_size = 1;
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->children[0]->str_size = 0;
                exp_node->children[1]->children[0]->num_value = 8;
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // bit b, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node(NODE_TYPE_EXPRESSION_8, NULL);            
                exp_node->str_value = "+";
                exp_node->str_size = 1;
                exp_node->children_count = 2;
                exp_node->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->str_size = 0;
                exp_node->children[0]->num_value = 0x46;
                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_value = "*";
                exp_node->children[1]->str_size = 1;
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->children[0]->str_size = 0;
                exp_node->children[1]->children[0]->num_value = 8;
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_r_value(node))
        {
            // bit b, r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node(NODE_TYPE_EXPRESSION_8, NULL);
                exp_node->str_value = "+";
                exp_node->str_size = 1;
                exp_node->children_count = 2;
                exp_node->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->str_value = "+";
                exp_node->children[0]->str_size = 1;
                exp_node->children[0]->children_count = 2;
                
                exp_node->children[0]->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->children[0]->str_size = 0;
                exp_node->children[0]->children[0]->num_value = 0x40;
                exp_node->children[0]->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->children[1]->str_value = "*";
                exp_node->children[0]->children[1]->str_size = 1;
                exp_node->children[0]->children[1]->children_count = 2;
                exp_node->children[0]->children[1]->children[0] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[0]->children[1]->children[0]->str_size = 0;
                exp_node->children[0]->children[1]->children[0]->num_value = 8;
                exp_node->children[0]->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[0]->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;

                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_size = 0;
                exp_node->children[1]->num_value = get_op_operand2_r_value(node);
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "call"))
    {
        if (is_op_operand1_number(node) && get_op_operand_count(node) == 1)
        {
            // call nn
            if (add_to_output)
            {
                add_output_element(0xCD, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "c") && is_op_operand2_number(node))
        {
            // call c, nn
            if (add_to_output)
            {
                add_output_element(0xDC, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "m") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xFC, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nc") && is_op_operand2_number(node))
        {
            if (add_to_output)
            {
                add_output_element(0xD4, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nz") && is_op_operand2_number(node))
        {
            if (add_to_output)
            {
                add_output_element(0xC4, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "p") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xF4, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "pe") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xEC, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "po") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xE4, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "z") && is_op_operand2_number(node))
        {
            if (add_to_output)
            {
                add_output_element(0xCC, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
    }
    else if (is_op_name(node, "ccf"))
    {
        // ccf
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0x3F, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "cp"))
    {
        if (is_op_operand1_paren_register(node, "hl") && get_op_operand_count(node) == 1)
        {
            // cp (hl)
            if (add_to_output)
            {
                add_output_element(0xBE, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "ix") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xBE, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "iy") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xBE, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_number(node) && get_op_operand_count(node) == 1)
        {
            // cp n
            if (add_to_output)
            {
                add_output_element(0xFE, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_r_value(node) && get_op_operand_count(node) == 1)
        {
            // cp r
            if (add_to_output)
            {
                add_output_element(0xB8 + get_op_operand1_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xB8 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xB8 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xB8 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // cp iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xB8 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "cpd"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xA9, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "cpdr"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xB9, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "cpi"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xA1, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "cpir"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xB1, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "cpl"))
    {
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0x2F, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "daa"))
    {
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0x27, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "dec"))
    {
        if (is_op_operand1_paren_register(node, "hl") && get_op_operand_count(node) == 1)
        {
            // dec (hl)
            if (add_to_output)
            {
                add_output_element(0x35, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "ix") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x35, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "iy") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x35, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && get_op_operand_count(node) == 1)
        {
            // dec a
            if (add_to_output)
            {
                add_output_element(0x3D, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && get_op_operand_count(node) == 1)
        {
            // dec b
            if (add_to_output)
            {
                add_output_element(0x05, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "bc") && get_op_operand_count(node) == 1)
        {
            // dec bc
            if (add_to_output)
            {
                add_output_element(0x0B, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && get_op_operand_count(node) == 1)
        {
            // dec c
            if (add_to_output)
            {
                add_output_element(0x0D, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && get_op_operand_count(node) == 1)
        {
            // dec d
            if (add_to_output)
            {
                add_output_element(0x15, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "de") && get_op_operand_count(node) == 1)
        {
            // dec de
            if (add_to_output)
            {
                add_output_element(0x1B, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && get_op_operand_count(node) == 1)
        {
            // dec e
            if (add_to_output)
            {
                add_output_element(0x1D, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && get_op_operand_count(node) == 1)
        {
            // dec h
            if (add_to_output)
            {
                add_output_element(0x25, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && get_op_operand_count(node) == 1)
        {
            // dec hl
            if (add_to_output)
            {
                add_output_element(0x2B, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x2B, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x2B, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x05 + 8 * 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x05 + 8 * 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x05 + 8 * 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // dec iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x05 + 8 * 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && get_op_operand_count(node) == 1)
        {
            // dec l
            if (add_to_output)
            {
                add_output_element(0x2D, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && get_op_operand_count(node) == 1)
        {
            // dec sp
            if (add_to_output)
            {
                add_output_element(0x3B, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "di"))
    {
        // di
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0xF3, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "djnz") && cpu_type != CPU_TYPE_GB)
    {
        // djnz n
        if (is_op_operand1_number(node))
        {
            if (add_to_output)
            {
                add_output_element(0x10, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ei"))
    {
        // ei
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0xFB, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "ex") && cpu_type != CPU_TYPE_GB)
    {
        if (is_op_operand1_paren_register(node, "sp") && is_op_operand2_register(node, "hl") && cpu_type != CPU_TYPE_GB)
        {
            // ex (sp), hl
            if (add_to_output)
            {
                add_output_element(0xE3, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_register(node, "sp") && is_op_operand2_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ex (sp), ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xE3, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_paren_register(node, "sp") && is_op_operand2_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ex (sp), iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xE3, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "af") && is_op_operand2_register(node, "af'") && cpu_type != CPU_TYPE_GB)
        {
            // ex af, af'
            if (add_to_output)
            {
                add_output_element(0x08, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "de") && is_op_operand2_register(node, "hl") && cpu_type != CPU_TYPE_GB)
        {
            // ex de, hl'
            if (add_to_output)
            {
                add_output_element(0xEB, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "exx"))
    {
        // exx
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0xD9, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "halt"))
    {
        // halt
        if (get_op_operand_count(node) == 0)
        {
            if (add_to_output)
            {
                add_output_element(0x76, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "im"))
    {
        // im n
        if (get_op_operand_count(node) == 1 && is_op_operand1_number(node) && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                struct ASTNode *match_list = create_node(NODE_TYPE_MATCH_LIST, NULL);
                match_list->children_count = 2;
                match_list->num_value = 0;
                match_list->num_value2 = 0x46;
                match_list->children[1] = exp_node;
                match_list->children[0] = create_node(NODE_TYPE_MATCH_LIST, NULL);
                match_list->children[0]->children_count = 1;
                match_list->children[0]->num_value = 1;
                match_list->children[0]->num_value2 = 0x56;
                match_list->children[0]->children[0] = create_node(NODE_TYPE_MATCH_LIST, NULL);
                match_list->children[0]->children[0]->children_count = 0;
                match_list->children[0]->children[0]->children[0] = NULL;
                match_list->children[0]->children[0]->num_value = 2;
                match_list->children[0]->children[0]->num_value2 = 0x5E;
                add_output_element(0, match_list);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "in") && cpu_type != CPU_TYPE_GB)
    {
        if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "c"))
        {
            // in a, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x78, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_number(node))
        {
            // in a, (n)
            if (add_to_output)
            {
                add_output_element(0xDB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_paren_register(node, "c"))
        {
            // in b, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x40, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_paren_register(node, "c"))
        {
            // in b, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x48, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_paren_register(node, "c"))
        {
            // in d, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x50, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_paren_register(node, "c"))
        {
            // in e, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x58, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_paren_register(node, "c"))
        {
            // in h, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x60, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_paren_register(node, "c"))
        {
            // in l, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x68, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "f") && is_op_operand2_paren_register(node, "c"))
        {
            // in f, (c)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x70, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "inc"))
    {
        if (is_op_operand1_paren_register(node, "hl") && get_op_operand_count(node) == 1)
        {
            // inc (hl)
            if (add_to_output)
            {
                add_output_element(0x34, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "ix") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x34, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "iy") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x34, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && get_op_operand_count(node) == 1)
        {
            // inc a
            if (add_to_output)
            {
                add_output_element(0x3C, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && get_op_operand_count(node) == 1)
        {
            // inc b
            if (add_to_output)
            {
                add_output_element(0x04, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "bc") && get_op_operand_count(node) == 1)
        {
            // inc bc
            if (add_to_output)
            {
                add_output_element(0x03, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && get_op_operand_count(node) == 1)
        {
            // inc c
            if (add_to_output)
            {
                add_output_element(0x0C, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && get_op_operand_count(node) == 1)
        {
            // inc d
            if (add_to_output)
            {
                add_output_element(0x14, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "de") && get_op_operand_count(node) == 1)
        {
            // inc de
            if (add_to_output)
            {
                add_output_element(0x13, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && get_op_operand_count(node) == 1)
        {
            // inc e
            if (add_to_output)
            {
                add_output_element(0x1C, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && get_op_operand_count(node) == 1)
        {
            // inc h
            if (add_to_output)
            {
                add_output_element(0x24, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && get_op_operand_count(node) == 1)
        {
            // inc hl
            if (add_to_output)
            {
                add_output_element(0x23, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x23, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x23, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x04 + 8 * 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x04 + 8 * 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyl") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x04 + 8 * 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyh") && get_op_operand_count(node) == 1 && cpu_type != CPU_TYPE_GB)
        {
            // inc iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x04 + 8 * 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && get_op_operand_count(node) == 1)
        {
            // inc hl
            if (add_to_output)
            {
                add_output_element(0x2C, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && get_op_operand_count(node) == 1)
        {
            // inc sp
            if (add_to_output)
            {
                add_output_element(0x33, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "ind"))
    {
        // ind
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xAA, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "indr"))
    {
        // indr
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xBA, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ini"))
    {
        // ini
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xA2, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "inir"))
    {
        // inir
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xB2, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "jp"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // jp n
            if (add_to_output)
            {
                add_output_element(0xC3, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "z") && is_op_operand2_number(node))
        {
            // jp z, n
            if (add_to_output)
            {
                add_output_element(0xCA, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nz") && is_op_operand2_number(node))
        {
            // jp nz, n
            if (add_to_output)
            {
                add_output_element(0xC2, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "c") && is_op_operand2_number(node))
        {
            // jp c, n
            if (add_to_output)
            {
                add_output_element(0xDA, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nc") && is_op_operand2_number(node))
        {
            // jp nc, n
            if (add_to_output)
            {
                add_output_element(0xD2, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "p") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // jp p, n
            if (add_to_output)
            {
                add_output_element(0xF2, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "m") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // jp m, n
            if (add_to_output)
            {
                add_output_element(0xFA, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "pe") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // jp pe, n
            if (add_to_output)
            {
                add_output_element(0xEA, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_cond(node, "po") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // jp po, n
            if (add_to_output)
            {
                add_output_element(0xE2, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
    }
    else if (is_op_name(node, "jr"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // jr n
            if (add_to_output)
            {
                add_output_element(0x18, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_cond(node, "c") && is_op_operand2_number(node))
        {
            // jr c, n
            if (add_to_output)
            {
                add_output_element(0x38, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nc") && is_op_operand2_number(node))
        {
            // jr nc, n
            if (add_to_output)
            {
                add_output_element(0x30, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_cond(node, "nz") && is_op_operand2_number(node))
        {
            // jr nz, n
            if (add_to_output)
            {
                add_output_element(0x20, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_cond(node, "z") && is_op_operand2_number(node))
        {
            // jr z, n
            if (add_to_output)
            {
                add_output_element(0x28, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ld"))
    {
        if (is_op_operand1_paren_register(node, "bc") && is_op_operand2_register(node, "a"))
        {
            // ld (bc), a
            if (add_to_output)
            {
                add_output_element(0x02, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_register(node, "de") && is_op_operand2_register(node, "a"))
        {
            // ld (de), a
            if (add_to_output)
            {
                add_output_element(0x12, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_register(node, "hl") && is_op_operand2_number(node))
        {
            // ld (hl), n
            if (add_to_output)
            {
                add_output_element(0x36, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_paren_register(node, "hl") && is_op_operand2_r_value(node))
        {
            // ld (hl), r
            if (add_to_output)
            {
                add_output_element(0x70 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "ix") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld (ix +/- o), n
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x36, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "ix") && is_op_operand2_r_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld (ix +/- o), r
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x70 + get_op_operand2_r_value(node), NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "iy") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld (iy +/- o), n
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x36, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_index_register(node, "iy") && is_op_operand2_r_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld (iy +/- o), r
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x70 + get_op_operand2_r_value(node), NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "a"))
        {
            // ld (n), a
            if (add_to_output)
            {
                if (cpu_type == CPU_TYPE_GB)
                {
                    add_output_element(0xEA, NULL);
                }
                else
                {                                
                    add_output_element(0x32, NULL);
                }
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "bc") && cpu_type != CPU_TYPE_GB)
        {
            // ld (n), bc
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x43, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "de") && cpu_type != CPU_TYPE_GB)
        {
            // ld (n), de
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x53, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "hl") && cpu_type != CPU_TYPE_GB)
        {
            // ld (n), hl
            if (add_to_output)
            {
                add_output_element(0x22, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld (n), ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x22, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld (n), iy
            if (add_to_output)
            {

                add_output_element(0xFD, NULL);
                add_output_element(0x22, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_paren_number(node) && is_op_operand2_register(node, "sp"))
        {
            // ld (n), sp
            if (add_to_output)
            {
                if (cpu_type == CPU_TYPE_GB)
                {
                    add_output_element(0x08, NULL);
                }
                else
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x73, NULL);
                }                
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = (cpu_type == CPU_TYPE_GB) ? 3 : 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "bc"))
        {
            // ld a, (bc)
            if (add_to_output)
            {
                add_output_element(0x0A, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "de"))
        {
            // ld a, (de)
            if (add_to_output)
            {
                add_output_element(0x1A, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld a, (hl)
            if (add_to_output)
            {
                add_output_element(0x7E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x7E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x7E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_number(node))
        {
            // ld a, (n)
            if (add_to_output)
            {
                if (cpu_type == CPU_TYPE_GB)
                {
                    add_output_element(0xFA, NULL);
                }
                else
                {
                    add_output_element(0x3A, NULL);
                }
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_number(node))
        {
            // ld a, n
            if (add_to_output)
            {
                add_output_element(0x3E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_r_value(node))
        {
            // ld a, r
            if (add_to_output)
            {
                add_output_element(0x78 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x78 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x78 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x78 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x78 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "i") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, i
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x57, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_register(node, "r") && cpu_type != CPU_TYPE_GB)
        {
            // ld a, r
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x5F, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_paren_register(node, "hl") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, (hl)
            if (add_to_output)
            {
                add_output_element(0x46, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x46, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x46, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_number(node))
        {
            // ld b, n
            if (add_to_output)
            {
                add_output_element(0x06, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_r_value(node))
        {
            // ld b, r
            if (add_to_output)
            {
                add_output_element(0x40 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x40 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x40 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x40 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "b") && is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // ld b, iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x40 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "bc") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld bc, (n)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x4B, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "bc") && is_op_operand2_number(node))
        {
            // ld bc, n
            if (add_to_output)
            {
                add_output_element(0x01, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld c, (hl)
            if (add_to_output)
            {
                add_output_element(0x4E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x4E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x4E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_number(node))
        {
            // ld c, n
            if (add_to_output)
            {
                add_output_element(0x0E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_r_value(node))
        {
            // ld c, r
            if (add_to_output)
            {
                add_output_element(0x48 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x48 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x48 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x48 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "c") && is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // ld c, iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x48 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld d, (hl)
            if (add_to_output)
            {
                add_output_element(0x56, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x56, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x56, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_number(node))
        {
            // ld d, n
            if (add_to_output)
            {
                add_output_element(0x16, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_r_value(node))
        {
            // ld d, r
            if (add_to_output)
            {
                add_output_element(0x50 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x50 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x50 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x50 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "d") && is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // ld d, iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x50 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "de") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld de, (n)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x5B, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "de") && is_op_operand2_number(node))
        {
            // ld de, n
            if (add_to_output)
            {
                add_output_element(0x11, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld e, (hl)
            if (add_to_output)
            {
                add_output_element(0x5E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x5E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x5E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_number(node))
        {
            // ld e, n
            if (add_to_output)
            {
                add_output_element(0x1E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_r_value(node))
        {
            // ld e, r
            if (add_to_output)
            {
                add_output_element(0x58 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x58 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x58 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x58 + 5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "e") && is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // ld e, iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x58 + 4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld h, (hl)
            if (add_to_output)
            {
                add_output_element(0x66, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld h, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x66, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld h, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x66, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_number(node))
        {
            // ld h, n
            if (add_to_output)
            {
                add_output_element(0x26, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "h") && is_op_operand2_r_value(node))
        {
            // ld h, r
            if (add_to_output)
            {
                add_output_element(0x60 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld hl, (n)
            if (add_to_output)
            {
                add_output_element(0x2A, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_number(node))
        {
            // ld hl, n
            if (add_to_output)
            {
                add_output_element(0x21, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "i") && is_op_operand2_register(node, "a") && cpu_type != CPU_TYPE_GB)
        {
            // ld i, a
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x47, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ix, (n)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x2A, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "ix") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ix, n
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x21, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixh") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ixh, n
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x26, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixh") && is_op_operand2_p_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ixh, p
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x60 + get_op_operand2_p_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixl") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ixl, n
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x2E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "ixl") && is_op_operand2_p_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld ixl, p
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x68 + get_op_operand2_p_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iy, (n)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x2A, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "iy") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iy, n
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x21, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyh") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iyh, n
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x26, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyh") && is_op_operand2_q_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iyh, q
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x60 + get_op_operand2_q_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyl") && is_op_operand2_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iyl, n
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x2E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "iyl") && is_op_operand2_q_value(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld iyl, q
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x68 + get_op_operand2_q_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_paren_register(node, "hl"))
        {
            // ld l, (hl)
            if (add_to_output)
            {
                add_output_element(0x6E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld l, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x6E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld l, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x6E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_number(node))
        {
            // ld l, n
            if (add_to_output)
            {
                add_output_element(0x2E, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "l") && is_op_operand2_r_value(node))
        {
            // ld l, r
            if (add_to_output)
            {
                add_output_element(0x68 + get_op_operand2_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "r") && is_op_operand2_register(node, "a") && cpu_type != CPU_TYPE_GB)
        {
            // ld r, a
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x4F, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_paren_number(node) && cpu_type != CPU_TYPE_GB)
        {
            // ld sp, (n)
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x7B, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_register(node, "hl"))
        {
            // ld sp, hl
            if (add_to_output)
            {
                add_output_element(0xF9, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // ld sp, ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xF9, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // ld sp, iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xF9, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "sp") && is_op_operand2_number(node))
        {
            // ld sp, n
            if (add_to_output)
            {
                add_output_element(0x31, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_16;
                add_output_element(0, exp_node);
                add_output_element(0, NULL);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "c"))
        {
            // ld a, (c)
            if (add_to_output)
            {
                add_output_element(0xF2, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand2_register(node, "a") && is_op_operand1_paren_register(node, "c"))
        {
            // ld (c), a
            if (add_to_output)
            {
                add_output_element(0xE2, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "ldd"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            // ldd
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xA8, NULL);
            }
            *length = 2;
            return 0;
        }
        else if(is_op_operand1_paren_register(node, "hl") && is_op_operand2_register(node, "a") && cpu_type == CPU_TYPE_GB)
        {
            // ldd (hl), a
            if (add_to_output)
            {
                add_output_element(0x32, NULL);
            }
            *length = 2;
            return 0;
        }
        else if(is_op_operand2_paren_register(node, "hl") && is_op_operand1_register(node, "a") && cpu_type == CPU_TYPE_GB)
        {
            // ldd a, (hl)
            if (add_to_output)
            {
                add_output_element(0x3A, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "lddr") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xB8, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "ldh") && cpu_type == CPU_TYPE_GB)
    {
        if (is_op_operand1_register(node, "a") && is_op_operand2_paren_number(node))
        {
            // ldh a, (n)
            if (add_to_output)
            {
                add_output_element(0xF0, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "a") && is_op_operand1_paren_number(node))
        {
            // ldh (n), a
            if (add_to_output)
            {
                add_output_element(0xE0, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ldhl") && cpu_type == CPU_TYPE_GB)
    {
        if (is_op_operand1_register(node, "sp") && is_op_operand2_number(node))
        {
            // ldh sp, n
            if (add_to_output)
            {
                add_output_element(0xF8, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ldi"))
    {
        if (get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
        {
            // ldi
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xA0, NULL);
            }
            *length = 2;
            return 0;
        }
        else if(is_op_operand1_paren_register(node, "hl") && is_op_operand2_register(node, "a") && cpu_type == CPU_TYPE_GB)
        {
            // ldi (hl), a
            if (add_to_output)
            {
                add_output_element(0x22, NULL);
            }
            *length = 2;
            return 0;
        }
        else if(is_op_operand2_paren_register(node, "hl") && is_op_operand1_register(node, "a") && cpu_type == CPU_TYPE_GB)
        {
            // ldi a, (hl)
            if (add_to_output)
            {
                add_output_element(0x2A, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ldir") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xB0, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "mulub") && cpu_type == CPU_TYPE_MSX)
    {
        if (is_op_operand1_register(node, "a") && is_op_operand2_r_value(node))
        {
            // mulub a, r
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                struct ASTNode *exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "+", 1);
                exp_node->children[0]->children_count = 2;
                exp_node->children[0]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0xC1);
                exp_node->children[0]->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[0]->children[1]->children_count = 2;
                exp_node->children[0]->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[0]->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[0]->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_size = 0;
                exp_node->children[1]->num_value = get_op_operand2_r_value(node);
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "muluw") && cpu_type == CPU_TYPE_MSX)
    {
        if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "bc"))
        {
            // muluw hl, bc
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xC3, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "hl") && is_op_operand2_register(node, "sp"))
        {
            // muluw hl, bc
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0xF3, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "neg") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0x44, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "nop") && get_op_operand_count(node) == 0)
    {
        if (add_to_output)
        {
            add_output_element(0x00, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "or"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // or (hl)
            if (add_to_output)
            {
                add_output_element(0xB6, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // or (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xB6, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // or (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xB6, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // or n
            if (add_to_output)
            {
                add_output_element(0xF6, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // or r
            if (add_to_output)
            {
                add_output_element(0xB0 + get_op_operand1_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // or ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xB0+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // or ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xB0+4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // or iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xB0+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // or iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xB0+4, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "otdr") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // otdr
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xBB, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "otir") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // otdr
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xB3, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "out") && cpu_type != CPU_TYPE_GB)
    {
        if (is_op_operand2_register(node, "a") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), a
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x79, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "b") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), b
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x41, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "c") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), c
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x49, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "d") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), d
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x51, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "e") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), e
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x59, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "h") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), h
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x61, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "l") && is_op_operand1_paren_register(node, "c"))
        {
            // out (c), l
            if (add_to_output)
            {
                add_output_element(0xED, NULL);
                add_output_element(0x69, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand2_register(node, "a") && is_op_operand1_paren_number(node))
        {
            // out (n), a
            if (add_to_output)
            {
                add_output_element(0xD3, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "outd") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // outd
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xAB, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "outi") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // outi
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0xA3, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "pop"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "af"))
        {
            // pop af
            if (add_to_output)
            {
                add_output_element(0xF1, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "bc"))
        {
            // pop bc
            if (add_to_output)
            {
                add_output_element(0xC1, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "de"))
        {
            // pop de
            if (add_to_output)
            {
                add_output_element(0xD1, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "hl"))
        {
            // pop hl
            if (add_to_output)
            {
                add_output_element(0xE1, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // pop ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xE1, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // pop iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xE1, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "push"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "af"))
        {
            // push af
            if (add_to_output)
            {
                add_output_element(0xF5, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "bc"))
        {
            // push bc
            if (add_to_output)
            {
                add_output_element(0xC5, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "de"))
        {
            // push de
            if (add_to_output)
            {
                add_output_element(0xD5, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "hl"))
        {
            // push hl
            if (add_to_output)
            {
                add_output_element(0xE5, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // push ix
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xE5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // push iy
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xE5, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "res"))
    {
        if (is_op_operand1_number(node) && is_op_operand2_paren_register(node, "hl"))
        {
            // res n, (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0x86);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // res n, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0x86);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // res n, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0x86);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (is_op_operand1_number(node) && is_op_operand2_r_value(node))
        {
            // res n, r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "+", 1);
                exp_node->children[0]->children_count = 2;
                exp_node->children[0]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0x80);
                exp_node->children[0]->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[0]->children[1]->children_count = 2;
                exp_node->children[0]->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[0]->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[0]->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                exp_node->children[1] = create_node(NODE_TYPE_EXPRESSION, NULL);
                exp_node->children[1]->str_size = 0;
                exp_node->children[1]->num_value = get_op_operand2_r_value(node);
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "ret"))
    {
        if (get_op_operand_count(node) == 0)
        {
            // ret
            if (add_to_output)
            {
                add_output_element(0xC9, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "c"))
        {
            // ret c
            if (add_to_output)
            {
                add_output_element(0xD8, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "m") && cpu_type != CPU_TYPE_GB)
        {
            // ret m
            if (add_to_output)
            {
                add_output_element(0xF8, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "nc"))
        {
            // ret nc
            if (add_to_output)
            {
                add_output_element(0xD0, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "nz"))
        {
            // ret nz
            if (add_to_output)
            {
                add_output_element(0xC0, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "p") && cpu_type != CPU_TYPE_GB)
        {
            // ret p
            if (add_to_output)
            {
                add_output_element(0xF0, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "pe") && cpu_type != CPU_TYPE_GB)
        {
            // ret pe
            if (add_to_output)
            {
                add_output_element(0xE8, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "po") && cpu_type != CPU_TYPE_GB)
        {
            // ret po
            if (add_to_output)
            {
                add_output_element(0xE0, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_cond(node, "z"))
        {
            // ret z
            if (add_to_output)
            {
                add_output_element(0xC8, NULL);
            }
            *length = 1;
            return 0;
        }
    }
    else if (is_op_name(node, "reti") && get_op_operand_count(node) == 0)
    {
        // reti
        if (add_to_output)
        {
            if (cpu_type == CPU_TYPE_GB)
            {
                add_output_element(0xD9, NULL);
            }
            else
            {
                add_output_element(0xED, NULL);
                add_output_element(0x4D, NULL);
            }
        }
        *length = (cpu_type == CPU_TYPE_GB) ? 1 : 2;
        return 0;
    }
    else if (is_op_name(node, "retn") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // retn
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0x45, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "rl"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // rl (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x16, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // rl (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x16, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // rl (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x16, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // rl r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x10 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "rla") && get_op_operand_count(node) == 0)
    {
        // rla
        if (add_to_output)
        {
            add_output_element(0x17, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "rlc"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // rlc (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x06, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // rlc (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x06, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // rlc (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x06, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // rlc r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "rlca") && get_op_operand_count(node) == 0)
    {
        // rlca
        if (add_to_output)
        {
            add_output_element(0x07, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "rld") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // rld
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0x6F, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "rr"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // rr (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x1E, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // rr (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x1E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // rr (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x1E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // rr r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x18 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "rra") && get_op_operand_count(node) == 0)
    {
        // rra
        if (add_to_output)
        {
            add_output_element(0x1F, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "rrc"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // rrc (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x0E, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // rrc (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x0E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // rrc (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x0E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // rrc r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x08 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "rrca") && get_op_operand_count(node) == 0)
    {
        // rrca
        if (add_to_output)
        {
            add_output_element(0x0F, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "rrd") && get_op_operand_count(node) == 0 && cpu_type != CPU_TYPE_GB)
    {
        // rrd
        if (add_to_output)
        {
            add_output_element(0xED, NULL);
            add_output_element(0x67, NULL);
        }
        *length = 2;
        return 0;
    }
    else if (is_op_name(node, "rst"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // rst n
            if (add_to_output)
            {
                struct ASTNode *exp_node = get_op_operand1_number(node);
                struct ASTNode *match_list = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x00, 0xC7);
                match_list->children_count = 2;
                match_list->children[1] = exp_node;
                match_list->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x08, 0xCF);
                match_list->children[0]->children_count = 1;
                match_list->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x10, 0xD7);
                match_list->children[0]->children[0]->children_count = 1;
                match_list->children[0]->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x18, 0xDF);
                match_list->children[0]->children[0]->children[0]->children_count = 1;
                match_list->children[0]->children[0]->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x20, 0xE7);
                match_list->children[0]->children[0]->children[0]->children[0]->children_count = 1;
                match_list->children[0]->children[0]->children[0]->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x28, 0xEF);
                match_list->children[0]->children[0]->children[0]->children[0]->children[0]->children_count = 1;
                match_list->children[0]->children[0]->children[0]->children[0]->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x30, 0xF7);
                match_list->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->children_count = 1;
                match_list->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0] = create_node_num2(NODE_TYPE_MATCH_LIST, NULL, 0x38, 0xFF);
                add_output_element(0, match_list);
            }
            *length = 1;
            return 0;
        }
    }
    if (is_op_name(node, "sbc"))
    {
        if (is_op_operand1_register(node, "a") && is_op_operand2_paren_register(node, "hl"))
        {
            // sbc a, (hl)
            if (add_to_output)
            {
                add_output_element(0x9E, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_number(node))
        {
            // sbc a, n
            if (add_to_output)
            {
                add_output_element(0xDE, NULL);
                struct ASTNode *exp_node = get_op_operand2_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // sbc a, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x9E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (is_op_operand1_register(node, "a") && is_op_operand2_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // sbc a, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x9E, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 2 && is_op_operand1_register(node, "a") && is_op_operand2_register_type(node))
        {
            // sbc a, r
            if (is_op_operand2_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xDD, NULL);
                    add_output_element(0x98 + 5, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xDD, NULL);
                    add_output_element(0x98 + 4, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xFD, NULL);
                    add_output_element(0x98 + 5, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xFD, NULL);
                    add_output_element(0x98 + 4, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_r_value(node))
            {
                if (add_to_output)
                {   
                    add_output_element(0x98 + get_op_operand2_r_value(node), NULL);
                }
                *length = 1;
                return 0;
            }
        }
        else if (get_op_operand_count(node) == 2 && is_op_operand1_register(node, "hl") && is_op_operand2_register_type(node))
        {
            // sbc hl, r
            if (is_op_operand2_register(node, "bc") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x42, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "de") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x52, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "hl") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x62, NULL);
                }
                *length = 2;
                return 0;
            }
            else if (is_op_operand2_register(node, "sp") && cpu_type != CPU_TYPE_GB)
            {
                if (add_to_output)
                {
                    add_output_element(0xED, NULL);
                    add_output_element(0x72, NULL);
                }
                *length = 2;
                return 0;
            }
        }
    }
    else if (is_op_name(node, "scf") && get_op_operand_count(node) == 0)
    {
        // scf
        if (add_to_output)
        {
            add_output_element(0x37, NULL);
        }
        *length = 1;
        return 0;
    }
    else if (is_op_name(node, "set"))
    {
        if (get_op_operand_count(node) == 2 && is_op_operand1_number(node) && is_op_operand2_paren_register(node, "hl"))
        {
            // res n, (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0xC6);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 2 && is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "ix"))
        {
            // set n, (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0xC6);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 2 && is_op_operand1_number(node) && is_op_operand2_paren_index_register(node, "iy"))
        {
            // set n, (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand2_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0xC6);
                exp_node->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[1]->children_count = 2;
                exp_node->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 2 && is_op_operand1_number(node) && is_op_operand2_r_value(node))
        {
            // set n, r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = create_node_str(NODE_TYPE_EXPRESSION_8, NULL, "+", 1);
                exp_node->children_count = 2;
                exp_node->children[0] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "+", 1);
                exp_node->children[0]->children_count = 2;
                exp_node->children[1] = create_node_num(NODE_TYPE_EXPRESSION, NULL, get_op_operand2_r_value(node));
                exp_node->children[0]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 0xC0);
                exp_node->children[0]->children[1] = create_node_str(NODE_TYPE_EXPRESSION, NULL, "*", 1);
                exp_node->children[0]->children[1]->children_count = 2;
                exp_node->children[0]->children[1]->children[0] = create_node_num(NODE_TYPE_EXPRESSION, NULL, 8);
                exp_node->children[0]->children[1]->children[1] = get_op_operand1_number(node);
                exp_node->children[0]->children[1]->children[1]->type = NODE_TYPE_EXPRESSION_3;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "sla"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // sla (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x26, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // sla (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x26, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // sla (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x26, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // sla r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x20 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "sll") && cpu_type == CPU_TYPE_Z80)
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // sll (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x36, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // sll (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x36, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // sll (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x36, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // sll r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x30 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "sra"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // sra (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x2E, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // sra (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x2E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // sra (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x2E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // sra r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x28 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "srl"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // srl (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x3E, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // srl (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x3E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // srl (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xCB, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
                add_output_element(0x3E, NULL);
            }
            *length = 4;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // srl r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x38 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "stop") && cpu_type == CPU_TYPE_GB)
    {
        if (get_op_operand_count(node) == 0)
        {
            add_output_element(0x10, NULL);
        }
    }
    else if (is_op_name(node, "sub"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // sub (hl)
            if (add_to_output)
            {
                add_output_element(0x96, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // sub (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x96, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // sub (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x96, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // sub n
            if (add_to_output)
            {
                add_output_element(0xD6, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // sub r
            if (add_to_output)
            {
                add_output_element(0x90 + get_op_operand1_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // sub ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x90+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // sub ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0x90+4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // sub iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x90+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // sub iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0x90+4, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "swap") && cpu_type == CPU_TYPE_GB)
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // swap (hl)
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x36, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // swap r
            if (add_to_output)
            {
                add_output_element(0xCB, NULL);
                add_output_element(0x30 + get_op_operand1_r_value(node), NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "xor"))
    {
        if (get_op_operand_count(node) == 1 && is_op_operand1_paren_register(node, "hl"))
        {
            // xor (hl)
            if (add_to_output)
            {
                add_output_element(0xAE, NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "ix") && cpu_type != CPU_TYPE_GB)
        {
            // xor (ix +/- o)
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xAE, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_paren_index_register(node, "iy") && cpu_type != CPU_TYPE_GB)
        {
            // xor (iy +/- o)
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xAE, NULL);
                struct ASTNode *exp_node = get_op_operand1_paren_index_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 3;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_number(node))
        {
            // xor n
            if (add_to_output)
            {
                add_output_element(0xEE, NULL);
                struct ASTNode *exp_node = get_op_operand1_number(node);
                exp_node->type = NODE_TYPE_EXPRESSION_8;
                add_output_element(0, exp_node);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_r_value(node))
        {
            // xor r
            if (add_to_output)
            {
                add_output_element(0xA8 + get_op_operand1_r_value(node), NULL);
            }
            *length = 1;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixl") && cpu_type != CPU_TYPE_GB)
        {
            // xor ixl
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xA8+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "ixh") && cpu_type != CPU_TYPE_GB)
        {
            // xor ixh
            if (add_to_output)
            {
                add_output_element(0xDD, NULL);
                add_output_element(0xA8+4, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyl") && cpu_type != CPU_TYPE_GB)
        {
            // xor iyl
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xA8+5, NULL);
            }
            *length = 2;
            return 0;
        }
        else if (get_op_operand_count(node) == 1 && is_op_operand1_register(node, "iyh") && cpu_type != CPU_TYPE_GB)
        {
            // xor iyh
            if (add_to_output)
            {
                add_output_element(0xFD, NULL);
                add_output_element(0xA8+4, NULL);
            }
            *length = 2;
            return 0;
        }
    }
    else if (is_op_name(node, "db"))
    {
        BOOL fail = FALSE;
        for(int i = 0; i < node->children_count; i++)
        {
            if (is_op_operandn_number(node, i))
            {
                struct ASTNode *exp_node = get_op_operandn_number(node, i);
                if (add_to_output)
                {
                    add_output_element_set_address(0, exp_node);
                }
            }
            else
            {
                fail = TRUE;
                break;
            }
        }
        if (!fail)
        {
            *length = node->children_count;
            return 0;
        }
    }

    write_compiler_error(node->filename, node->file_line, "Invalid combination of instruction \"%.*s\" and operands", node->str_size, node->str_value);
    return 1;
}

void fprint_operand_node(FILE *fp, struct ASTNode *node)
{
    switch(node->type)
    {
        case NODE_TYPE_EXPRESSION:
        case NODE_TYPE_EXPRESSION_8:
        case NODE_TYPE_EXPRESSION_3:
        case NODE_TYPE_EXPRESSION_32:
        case NODE_TYPE_EXPRESSION_8c:
        case NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS:
        case NODE_TYPE_EXPRESSION_16:
        {
            if (node->children_count == 0)
            {
                if (node->str_size > 0)
                {
                    if (node->str_size2 > 0)
                    {
                        fprintf(fp, "%.*s::%.*s", node->str_size2, node->str_value2, node->str_size, node->str_value);
                    }
                    else
                    {
                        fprintf(fp, "%.*s", node->str_size, node->str_value);
                    }
                }
                else
                {
                    fprintf(fp, "%"PRId64"", node->num_value);
                }
            }
            else if (node->children_count == 1)
            {                
                fprintf(fp, "%.*s", node->str_size, node->str_value);
                if (is_str_equal(node->str_value, node->str_size, "sizeof") ||
                    is_str_equal(node->str_value, node->str_size, "length"))
                {
                    fprintf(fp, "(");
                    fprint_operand_node(fp, node->children[0]);
                    fprintf(fp, ")");
                }
                else
                {
                    fprint_operand_node(fp, node->children[0]);
                }
            }
            else if (node->children_count == 2)
            {   
                fprint_operand_node(fp, node->children[0]);             
                fprintf(fp, "%.*s", node->str_size, node->str_value);
                fprint_operand_node(fp, node->children[1]);
            }
            break;
        }
        case NODE_TYPE_INDEX_REGISTER:
        {
            fprint_operand_node(fp, node->children[0]);
            fprint_operand_node(fp, node->children[1]);            
            break;
        }
        case NODE_TYPE_LPAREN:
        {
            fprintf(fp, "(");
            fprint_operand_node(fp, node->children[0]);
            fprintf(fp, ")");
            break;
        }
        default:
        {
            if (node->str_size2 > 0)
            {
                fprintf(fp, "%.*s::%.*s", node->str_size2, node->str_value2, node->str_size, node->str_value);
            }
            else
            {
                fprintf(fp, "%.*s", node->str_size, node->str_value);
            }
        }
    }
}

void fprint_op(FILE *fp, struct ASTNode *node)
{
    fprintf(fp, "%.*s", node->str_size, node->str_value);
    for(int i = 0; i < node->children_count; i++)
    {
        if (i > 0)
        {
            fprintf(fp, ",");
        }
        fprintf(fp, " ");
        fprint_operand_node(fp, node->children[i]);
    }
    fprintf(fp, "\n");
}
