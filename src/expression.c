/*
    Copyright (c) 2023, Sérgio Vieira <internalregister@gmail.com>
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

static int get_size_and_length_of_identifier(struct ASTNode *identifier_node, int *size, int *length)
{
    if (identifier_node->type == NODE_TYPE_DATA_TYPE)
    {
        if (is_str_equal(identifier_node->str_value, identifier_node->str_size, "byte"))
        {
            *size = 1;
        }
        else if (is_str_equal(identifier_node->str_value, identifier_node->str_size, "word"))
        {
            *size = 2;
        }
        else if (is_str_equal(identifier_node->str_value, identifier_node->str_size, "dword"))
        {
            *size = 4;
        }
        else
        {
            assert(1 == 0 && "Unexpected data type in sizeof expression");
        }

        *length = 1;

        return 0;
    }

    struct ASTNode *next_node = NULL;

    if (is_str_equal(identifier_node->str_value, identifier_node->str_size, "."))
    {
        next_node = identifier_node->children[1];
        identifier_node = identifier_node->children[0];
    }

    // Check data symbol
    struct DataSymbol *data_symbol = get_data_symbol(identifier_node->str_value, identifier_node->str_size, identifier_node->str_value2, identifier_node->str_size2);
    struct StructuredType *structured_type = NULL;
    if (data_symbol != NULL)
    {
        structured_type = data_symbol->structured_type;
        *size = data_symbol->size_of_type;
        *length = (data_symbol->length <= 0 ? 1 : data_symbol->length);
    }
    else
    {
        // Check structured type
        structured_type = get_structured_type(identifier_node->str_value, identifier_node->str_size, identifier_node->str_value2, identifier_node->str_size2);
        if (structured_type != NULL)
        {
            *size = structured_type->struct_size;
            *length = 1;
        }
        else
        {
            if (identifier_node->str_size2 == 0)
            {
                write_compiler_error(identifier_node->filename, identifier_node->file_line, "Data or type \"%.*s\" not found", identifier_node->str_size, identifier_node->str_value);
            }
            else
            {
                write_compiler_error(identifier_node->filename, identifier_node->file_line, "Data or type \"%.*s::%.*s\" not found", identifier_node->str_size2, identifier_node->str_value2, identifier_node->str_size, identifier_node->str_value);
            }
            return 1;
        }
    }

    if (next_node != NULL)
    {        
        do
        { 
            if (structured_type == NULL)
            {
                write_compiler_error(identifier_node->filename, identifier_node->file_line, "Unexpected \".\" in data symbol of not a structured type", 0);
                return 1;
            }        

            if (is_str_equal(next_node->str_value, next_node->str_size, "."))
            {
                identifier_node = next_node->children[0];
                next_node = next_node->children[1];
            }
            else
            {
                identifier_node = next_node;
                next_node = NULL;                
            }

            struct StructElement *structure_element = get_struct_element(structured_type, identifier_node->str_value, identifier_node->str_size);
            if (structure_element == NULL)
            {
                if (structured_type->library_name_size > 0)
                {
                    write_compiler_error(identifier_node->filename, identifier_node->file_line, "Element \"%.*s\" not found in structured type \"%.*s\"", identifier_node->str_size, identifier_node->str_value, structured_type->library_name, structured_type->library_name_size, structured_type->name, structured_type->name_size);
                    return 1;
                }
                else
                {
                    write_compiler_error(identifier_node->filename, identifier_node->file_line, "Element \"%.*s\" not found in structured type \"%.*s\"", identifier_node->str_size, identifier_node->str_value, structured_type->name, structured_type->name_size);
                    return 1;
                }
            }
            *size = structure_element->size_of_type;
            *length = (structure_element->array_length <= 0 ? 1 : structure_element->array_length);
            structured_type = structure_element->structured_type;
        } while (next_node != NULL);
    }

    return 0;
}

int resolve_expression(struct ASTNode *node, int64_t *result)
{
    int64_t temp1, temp2;
    if (!is_node_expression(node))
    {
        write_compiler_error(node->filename, node->file_line, "Invalid expression", 0);
        return 1;
    }

    if (node->str_size == 0)
    {
        *result = node->num_value;
    }
    else
    {
        switch(node->str_value[0])
        {
            case '+':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (node->children_count == 1)
                {
                    *result = temp1;
                    break;
                }
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 + temp2;
                break;
            }
            case '-':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (node->children_count == 1)
                {
                    *result = -temp1;
                    break;
                }
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 - temp2;
                break;
            }
            case '*':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 * temp2;
                break;
            }
            case '/':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 / temp2;
                break;
            }
            case '%':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 % temp2;
                break;
            }
            case '&':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 & temp2;
                break;
            }
            case '|':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 | temp2;
                break;
            }
            case '~':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                *result = ~temp1;
                break;
            }
            case '^':
            {
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 ^ temp2;
                break;
            }
            case '<':
            {
                assert(node->str_size == 2 && node->str_value[1] == '<');
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 << temp2;
                break;
            }
            case '>':
            {
                assert(node->str_size == 2 && node->str_value[1] == '>');
                if (resolve_expression(node->children[0], &temp1)) return 1;
                if (resolve_expression(node->children[1], &temp2)) return 1;
                *result = temp1 >> temp2;
                break;
            }
            case '$':
            {
                *result = (int64_t) compiler_current_address;
                break;
            }
            case '.':
            {
                BOOL clear_address = FALSE;
                if (is_str_equal(node->str_value, node->str_size, ".|"))
                {
                    clear_address = TRUE;
                }

                int64_t address = 0;

                struct StructuredType *structured_type = NULL;

                struct ASTNode *first_identifier_node = node->children[0];
                struct DataSymbol *data_symbol = get_data_symbol(first_identifier_node->str_value, first_identifier_node->str_size, first_identifier_node->str_value2, first_identifier_node->str_size2);
                if (data_symbol != NULL)
                {
                    get_constant(first_identifier_node->str_value2, first_identifier_node->str_size2, first_identifier_node->str_value, first_identifier_node->str_size, &address);

                    if (data_symbol->is_native_type)
                    {
                        if (first_identifier_node->str_size2 == 0)
                        {
                            write_compiler_error(first_identifier_node->filename, first_identifier_node->file_line, "Data symbol \"%.*s\" is not a structure", first_identifier_node->str_size, first_identifier_node->str_value);
                        }
                        else
                        {
                            write_compiler_error(first_identifier_node->filename, first_identifier_node->file_line, "Data symbol \"%.*s::%.*s\" is not a structure", first_identifier_node->str_size2, first_identifier_node->str_value2, first_identifier_node->str_size, first_identifier_node->str_value);
                        }
                        return 1;
                    }

                    structured_type = data_symbol->structured_type;

                    if (first_identifier_node->children_count == 1 && first_identifier_node->children[0]->type == NODE_TYPE_INDEX)
                    {
                        int64_t index;
                        resolve_expression(first_identifier_node->children[0]->children[0], &index);
                        if (index < -1 || index > data_symbol->length - 1)
                        {
                            write_compiler_error(first_identifier_node->filename, first_identifier_node->file_line, "Invalid index %"PRId64" for data symbol \"%.*s\"", index, first_identifier_node->str_size, first_identifier_node->str_value);
                            return 1;
                        }
                        address += index * structured_type->struct_size;
                    }
                }
                else
                {
                    structured_type = get_structured_type(first_identifier_node->str_value, first_identifier_node->str_size, first_identifier_node->str_value2, first_identifier_node->str_size2);
                    if (structured_type == NULL)
                    {
                        if (first_identifier_node->str_size2 == 0)
                        {
                            write_compiler_error(first_identifier_node->filename, first_identifier_node->file_line, "Data symbol or type \"%.*s\" not found", first_identifier_node->str_size, first_identifier_node->str_value);
                        }
                        else
                        {
                            write_compiler_error(first_identifier_node->filename, first_identifier_node->file_line, "Data symbol or type \"%.*s::%.*s\" not found", first_identifier_node->str_size2, first_identifier_node->str_value2, first_identifier_node->str_size, first_identifier_node->str_value);
                        }
                        return 1;
                    }
                }
                
                struct StructElement *struct_element = NULL;
                struct ASTNode *current_expression_node = node->children[1], *current_identifier_node = NULL;
                BOOL leave = TRUE;
                do
                {
                    if (clear_address)
                    {
                        address = 0;
                        clear_address = FALSE;
                    }

                    if (is_str_equal(current_expression_node->str_value, current_expression_node->str_size, "."))
                    {
                        current_identifier_node = current_expression_node->children[0];
                        leave = FALSE;
                    }
                    else if (is_str_equal(current_expression_node->str_value, current_expression_node->str_size, ".|"))
                    {
                        current_identifier_node = current_expression_node->children[0];                        
                        clear_address = TRUE;
                        leave = FALSE;
                    }
                    else
                    {
                        current_identifier_node = current_expression_node;
                        leave = TRUE;
                    }

                    struct_element = get_struct_element(structured_type, current_identifier_node->str_value, current_identifier_node->str_size);

                    if (struct_element == NULL)
                    {
                        if (structured_type->library_name_size > 0)
                        {
                            write_compiler_error(current_identifier_node->filename, current_identifier_node->file_line, "Struct element \"%.*s\" not in structure \"%.*s::%.*s\"",
                                current_identifier_node->str_size, current_identifier_node->str_value,
                                structured_type->library_name_size, structured_type->library_name,
                                structured_type->name_size, structured_type->name);
                        }
                        else
                        {
                            write_compiler_error(current_identifier_node->filename, current_identifier_node->file_line, "Struct element \"%.*s\" not in structure \"%.*s\"",
                                current_identifier_node->str_size, current_identifier_node->str_value,
                                structured_type->name_size, structured_type->name);
                        }

                        return 1;
                    }

                    address += struct_element->position;

                    if (current_identifier_node->children_count == 1 && current_identifier_node->children[0]->type == NODE_TYPE_INDEX)
                    {                        
                        int64_t index;
                        resolve_expression(current_identifier_node->children[0]->children[0], &index);
                        if (index < -1 || index > struct_element->array_length - 1)
                        {
                            write_compiler_error(current_identifier_node->filename, current_identifier_node->file_line, "Invalid index %"PRId64" for struct element \"%.*s\"",
                                index, current_identifier_node->str_size, current_identifier_node->str_value);
                            return 1;
                        }

                        if (struct_element->structured_type != NULL)
                        {
                            address += index * struct_element->structured_type->struct_size;
                        }
                        else
                        {
                            address += index * get_native_type_size(struct_element->type, struct_element->type_size);
                        }
                    }

                    if (leave)
                    {
                        break;
                    }    

                    if (struct_element->structured_type == NULL)
                    {
                        if (structured_type->library_name_size > 0)
                        {
                            write_compiler_error(current_identifier_node->filename, current_identifier_node->file_line, "Struct element \"%.*s\" from structure \"%.*s::%.*s\" is not of a structure",
                                current_identifier_node->str_size, current_identifier_node->str_value,
                                structured_type->library_name_size, structured_type->library_name,
                                structured_type->name_size, structured_type->name);
                        }
                        else
                        {
                            write_compiler_error(current_identifier_node->filename, current_identifier_node->file_line, "Struct element \"%.*s\" from structure \"%.*s\" is not a structure",
                                current_identifier_node->str_size, current_identifier_node->str_value,
                                structured_type->name_size, structured_type->name);
                        }

                        return 1;
                    }

                    structured_type = struct_element->structured_type;

                    current_expression_node = current_expression_node->children[1];
                } while (TRUE);
                
                *result = address;
                break;
            }
            default:
            {
                if (is_str_equal(node->str_value, node->str_size, "sizeof") && is_str_equal(node->str_value2, node->str_size2, ""))
                {
                    struct ASTNode *identifier_node = node->children[0];

                    int length, size;

                    if (get_size_and_length_of_identifier(identifier_node, &size, &length))
                    {
                        write_compiler_error(identifier_node->filename, identifier_node->file_line, "Error in sizeof expression", 0);
                        return 1;
                    }

                    *result = size * length;
                }
                else if (is_str_equal(node->str_value, node->str_size, "length") && is_str_equal(node->str_value2, node->str_size2, ""))
                {
                    struct ASTNode *identifier_node = node->children[0];

                    int length, size;

                    if (get_size_and_length_of_identifier(identifier_node, &size, &length))
                    {
                        write_compiler_error(identifier_node->filename, identifier_node->file_line, "Error in length expression", 0);
                        return 1;
                    }

                    *result = length;
                }
                else
                {
                    // Identifier

                    if (get_constant(node->str_value2, node->str_size2, node->str_value, node->str_size, &temp1))
                    {
                        if (node->str_size2 == 0)
                        {
                            write_compiler_error(node->filename, node->file_line, "Symbol \"%.*s\" not found", node->str_size, node->str_value);
                        }
                        else
                        {
                            write_compiler_error(node->filename, node->file_line, "Symbol \"%.*s::%.*s\" not found", node->str_size2, node->str_value2, node->str_size, node->str_value);
                        }
                        return 1;
                    }

                    if (node->children_count == 1 && node->children[0]->type == NODE_TYPE_INDEX)
                    {
                        struct DataSymbol *data_symbol = get_data_symbol(node->str_value, node->str_size, node->str_value2, node->str_size2);
                        if (data_symbol == NULL)
                        {
                            if (node->str_size2 == 0)
                            {
                                write_compiler_error(node->filename, node->file_line, "Indexed data symbol \"%.*s\" not found", node->str_size, node->str_value);
                            }
                            else
                            {
                                write_compiler_error(node->filename, node->file_line, "Indexed data symbol \"%.*s::%.*s\" not found", node->str_size2, node->str_value2, node->str_size, node->str_value);
                            }
                            return 1;
                        }
                        int64_t index;
                        resolve_expression(node->children[0]->children[0], &index);
                        if (index < -1 || index > data_symbol->length - 1)
                        {
                            write_compiler_error(node->filename, node->file_line, "Invalid index %"PRId64" for data symbol \"%.*s\"", index, node->str_size, node->str_value);
                            return 1;
                        }
                        temp1 += data_symbol->size_of_type * index;
                    }
                    *result = temp1;
                }
                break;
            }
        }
    }

    if (node->type == NODE_TYPE_EXPRESSION_8)
    {
        if (*result > UINT8_MAX || *result < INT8_MIN)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 8-bit value", *result);
            return 1;
        }
    }
    else if (node->type == NODE_TYPE_EXPRESSION_8c)
    {
        if (*result > UINT8_MAX || *result < INT8_MIN)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 8-bit value", *result);
            return 1;
        }
        *result = *result - 2;        
    }
    else if (node->type == NODE_TYPE_EXPRESSION_3)
    {
        if (*result > 7 || *result < 0)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 3-bit value", *result);
            return 1;
        }
    }
    else if (node->type == NODE_TYPE_EXPRESSION_16)
    {
        if (*result > UINT16_MAX || *result < INT16_MIN)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 16-bit value", *result);
            return 1;
        }
    }
    else if (node->type == NODE_TYPE_EXPRESSION_32)
    {
        if (*result > UINT32_MAX || *result < INT32_MIN)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 32-bit value", *result);
            return 1;
        }
    }
    else if (node->type == NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS)
    {
        *result = *result - compiler_current_address;
        if (*result > UINT8_MAX || *result < INT8_MIN)
        {
            write_compiler_error(node->filename, node->file_line, "Expression value %"PRId64" is an invalid 8-bit value", *result);
            return 1;
        }
    }

    return 0;
}
