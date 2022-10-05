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

BOOL is_str_equal(char *str1, int str1_size, char *str2)
{
    int c = 0;

    if (str1_size == 0 && str2[0] == '\0')
    {
        return TRUE;
    }
    else if (str1_size == 0)
    {
        return FALSE;
    }

    while(str1[c] == str2[c])
    {        
        c++;
        if (c == str1_size && str2[c] == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL is_str_equal2(char *str1, int str1_size, char *str2, int str2_size)
{
    int c = 0;

    if (str1_size != str2_size)
    {
        return FALSE;
    }

    if (str1_size == 0)
    {
        return TRUE;
    }

    while(str1[c] == str2[c])
    {
        c++;
        if (c == str1_size)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL is_native_type(char *type, int type_size)
{
    return (is_str_equal(type, type_size, "byte") ||
            is_str_equal(type, type_size, "word") ||
            is_str_equal(type, type_size, "dword"));
}

int get_native_type_size(char *type, int type_size)
{
    if (is_str_equal(type, type_size, "byte"))
    {
        return 1;
    }

    if (is_str_equal(type, type_size, "word"))
    {
        return 2;
    }

    if (is_str_equal(type, type_size, "dword"))
    {
        return 4;
    }

    return -1;
}

void filename_get_path(char *dst, char *filename)
{
    int position = 0, position_last_slash = 0;
    
    do
    {
        dst[position] = filename[position];
        if (dst[position] == '/' || dst[position] == '\\')
        {
            position_last_slash = position;
        }
        position++;
    } while (filename[position] != '\0');

    dst[position_last_slash] = '\0';
}

void filename_add_path(char *dst, char *filename, char *path)
{
    int count = 0;

    if (filename[0] != '/' && filename[0] != '\\' && filename[1] != ':' && path[0] != '\0')
    {
        // Filename doesn't have an absolute path
        while (path[0] != '\0')
        {
            dst[0] = path[0];
            dst++; path++;
            count++;
        }

        if (count > 0 && dst[-1] != '/' && dst[-1] != '\\')
        {
            dst[0] = '/';
            dst++;
        }
    }

    do
    {
        dst[0] = filename[0];
        dst++; filename++;
    } while (filename[0] != '\0');
    dst[0] = '\0';
}

BOOL is_node_expression_type(enum NodeType node_type)
{
    return node_type == NODE_TYPE_EXPRESSION || node_type == NODE_TYPE_EXPRESSION_8 ||
        node_type == NODE_TYPE_EXPRESSION_8c || node_type == NODE_TYPE_EXPRESSION_8_REL_CUR_ADDRESS ||
        node_type == NODE_TYPE_EXPRESSION_16 || node_type == NODE_TYPE_EXPRESSION_32 ||
        node_type == NODE_TYPE_EXPRESSION_3;
}
