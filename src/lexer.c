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

int init_lexer(struct Lexer *lexer, char *filename)
{
    char *content = NULL;

    content = get_content_from_cache(filename);

    if (content == NULL)
    {
        FILE *fp = fopen(filename, "r");
        if (!fp)
        {
            write_error("Can't open file \"%s\".", filename);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);        
        lexer->buffer_start = (char *)malloc((size_t)file_size + 1);
        size_t read_size = fread(lexer->buffer_start, sizeof(char), (size_t)file_size, fp);
        if (read_size != file_size)
        {
            write_error("Error reading file \"%s\".", filename);
            return 1;
        }
        write_debug("Opened file \"%s\" with the size of %ld bytes.", filename, file_size);
        lexer->buffer_start[file_size] = 0;
        lexer->buffer_at = lexer->buffer_start;
        fclose(fp);

        add_content_to_cache(filename, lexer->buffer_start);
    }
    else
    {
        write_debug("Using content from cache for file \"%s\"", filename);
        lexer->buffer_start = lexer->buffer_at = content;
    }

    lexer->filename = filename;
    lexer->current_line = 1;

    return 0;
}

int destroy_lexer(struct Lexer *lexer)
{
    free(lexer->buffer_start);

    return 0;
}

static int get_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline, BOOL rewind, BOOL skip_chars);

static int eat_tokens_until_endif(struct Lexer *lexer, struct Token *token, BOOL break_on_else)
{
    int nested_ifdefs = 0;
    do
    {
        if (get_token(lexer, token, FALSE, FALSE, FALSE)) { return 1; }

        // Eat nested #ifdef or #ifndef
        if (token->type == TOKEN_TYPE_IFDEF || token->type == TOKEN_TYPE_IFNDEF)
        {
            nested_ifdefs++;
        }
        else if (token->type == TOKEN_TYPE_IFDEF_ENDIF)
        {
            if (nested_ifdefs == 0)
            {
                break;
            }
            nested_ifdefs--;
        }
        else if (token->type == TOKEN_TYPE_IFDEF_ELSE && nested_ifdefs == 0 && break_on_else)
        {
            break;
        }
    } while (TRUE);

    return 0;
}

static int skip_characters(struct Lexer *lexer, BOOL skip_newline, BOOL rewind)
{
    int ifdef_push_count = 0;
    
    if (rewind)
    {
        save_duplicate_all_ifdef_expect();
    }

    while(TRUE)
    {
        while(lexer->buffer_at[0] == ' ' ||
            lexer->buffer_at[0] == '\t' ||
            lexer->buffer_at[0] == '\r' ||
            lexer->buffer_at[0] == ';' ||
            (lexer->buffer_at[0] == '/' && lexer->buffer_at[1] == '/') ||
            (lexer->buffer_at[0] == '/' && lexer->buffer_at[1] == '*') ||
            (skip_newline && lexer->buffer_at[0] == '\n'))
        {
            if (lexer->buffer_at[0] == '\n') { lexer->current_line++; }

            // Comments
            if ((lexer->buffer_at[0] == ';') ||
                (lexer->buffer_at[0] == '/' && lexer->buffer_at[1] == '/'))
            {
                do        
                {
                    lexer->buffer_at++;
                } while(lexer->buffer_at[0] != '\n' && lexer->buffer_at[0] != 0);
            }
            else if (lexer->buffer_at[0] == '/' && lexer->buffer_at[1] == '*')
            {
                // TODO: Support nested multi-line comments
                do
                {
                    if (lexer->buffer_at[0] == '\n') { lexer->current_line++; }
                    lexer->buffer_at++;
                } while(!(lexer->buffer_at[0] == '*' && lexer->buffer_at[1] == '/') && lexer->buffer_at[0] != 0);
                if (lexer->buffer_at[0] == '*') { lexer->buffer_at += 2; }
            }
            else
            {
                lexer->buffer_at++;
            }
        }

        struct Token token;
        if (get_token(lexer, &token, skip_newline, TRUE, FALSE)) { return 1; }
        if (token.type == TOKEN_TYPE_IFDEF || token.type == TOKEN_TYPE_IFNDEF)
        {
            BOOL check_defined = (token.type == TOKEN_TYPE_IFDEF);
            if (get_token(lexer, &token, skip_newline, FALSE, FALSE)) { return 1; }

            // read identifier
            if (get_next_token(lexer, &token, FALSE)) { return 1; }
            if (token.type != TOKEN_TYPE_IDENTIFIER)
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Expected an identifier of #ifdef or #ifndef, found '%.*s'", token.size, token.value);
                return 1;
            }

            // check identifier
            BOOL has_identifier = has_define_identifier(token.value, token.size);
            if ((has_identifier && check_defined) || (!has_identifier && !check_defined))
            {
                // ignore next #else and wait for #endif
                push_ifdef_expect(IFDEF_EXPECT_ELSE);
                ifdef_push_count++;
            }
            else
            {
                // wait until #else or #endif
                if (eat_tokens_until_endif(lexer, &token, TRUE)) { return 1; }

                if (token.type == TOKEN_TYPE_IFDEF_ELSE)
                {
                    // wait until #endif
                    push_ifdef_expect(IFDEF_EXPECT_ENDIF);
                    ifdef_push_count++;
                }
            }
        }
        else if (token.type == TOKEN_TYPE_IFDEF_ELSE)
        {
            if (peek_ifdef_expect() == IFDEF_EXPECT_ELSE)
            {
                if (get_token(lexer, &token, skip_newline, FALSE, FALSE)) { return 1; }

                pop_ifdef_expect();
                ifdef_push_count--;

                // wait until #endif
                if (eat_tokens_until_endif(lexer, &token, FALSE)) { return 1; }
            }
            else
            {
                break;
            }
        }
        else if (token.type == TOKEN_TYPE_IFDEF_ENDIF)
        {
            enum IfdefExpectType ifdef_expected_result = peek_ifdef_expect();
            if (ifdef_expected_result == IFDEF_EXPECT_ENDIF || ifdef_expected_result == IFDEF_EXPECT_ELSE)
            {
                if (get_token(lexer, &token, skip_newline, FALSE, FALSE)) { return 1; }

                pop_ifdef_expect();
                ifdef_push_count--;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    };    

    if (rewind)
    {
        revert_to_duplicate_ifdef_expect();
    }

    return 0;
}

static BOOL is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static BOOL is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

#define NUM_REGISTERS 23
static char *register_names[] = {
    "a", "b", "c", "d", "e", "h", "l", "f", "i", "r", "af", "bc", "de", "hl", "sp", "pc", "ix", "iy", "ixl", "ixh", "iyl", "iyh", "af'"
};

#define NUM_OPS 68
static char *op_names[] = {
    "nop", "adc", "add", "and", "bit", "call", "ccf", "cp", "cpd", "cpdr", "cpi", "cpir", "cpl", "daa", "dec", "di", "djnz", "ei", "ex", "exx", "halt", "im",
    "in", "inc", "ind", "indr", "ini", "inir", "jp", "jr", "ld", "ldd", "lddr", "ldi", "ldir", "neg", "or", "otdr", "otir", "out", "outd", "outi",
    "pop", "push", "res", "ret", "reti", "retn", "rl", "rla", "rlc", "rlca", "rld", "rr", "rra", "rrc", "rrca", "rrd", "rst", "sbc", "scf", "set",
    "sla", "sra", "srl", "sub", "xor", "db", "sll", "swap", "stop", "ldh", "ldhl", "mulub", "muluw"
};

#define NUM_CONDS 7
static char *cond_names[] = {
    "nc", "m", "p", "z", "nz", "pe", "po"
};

static BOOL is_str_equal_token_value(struct Token *token, char *str)
{
    return is_str_equal(token->value, token->size, str);
}

static BOOL is_token_in_list(struct Token *token, char *list[], int list_size)
{
    for(int i = 0; i < list_size; i++) { if (is_str_equal_token_value(token, list[i])) return TRUE; }
    return FALSE;
}

static int64_t str_to_number(char *str, int str_size, int base)
{
    int64_t result = 0, base_mul = 0;
    char c;

    if(base < 2 || base > 16)
    {
        return result;
    }    

    base_mul = 1;

    for(int i = str_size - 1; i >= 0; i--)
    {
        c = str[i];
        if (c >= 'a')
        {
            result += base_mul * (c - 'a' + 10);
        }
        else if (c >= 'A')
        {
            result += base_mul * (c - 'A' + 10);
        }
        else
        {
            result += base_mul * (c - '0');
        }

        base_mul *= base;
    }

    return result;
}

static BOOL get_escaped_character(char c, char *output)
{
    switch(c)
    {
        case '0':
        {
            *output = 0x00;
            break;
        }
        case 'a':
        {
            *output = 0x07;
            break;
        }
        case 'b':
        {
            *output = 0x08;
            break;
        }
        case 'e':
        {
            *output = 0x1B;
            break;
        }
        case 'f':
        {
            *output = 0x0C;
            break;
        }
        case 'n':
        {
            *output = 0x0A;
            break;
        }
        case 'r':
        {
            *output = 0x0D;
            break;
        }
        case 't':
        {
            *output = 0x09;
            break;
        }
        case 'v':
        {
            *output = 0x0B;
            break;
        }
        case '\\':
        {
            *output = '\\';
            break;
        }
        case '\'':
        {
            *output = '\'';
            break;
        }
        case '\"':
        {
            *output = '\"';
            break;
        }
        default:
        {
            return FALSE;
        }
    }

    return TRUE;
}

static int get_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline, BOOL rewind, BOOL skip_chars)
{
    char *buffer_previous_position = lexer->buffer_at;
    int previous_line = lexer->current_line;

    if (skip_chars)
    {
        skip_characters(lexer, skip_newline, rewind);
        // buffer_previous_position = lexer->buffer_at;
        // previous_line = lexer->current_line;
    }

    token->value = lexer->buffer_at;
    token->size = 1;
    token->size2 = 0;
    token->type = TOKEN_TYPE_INVALID;
    
    switch(lexer->buffer_at[0])
    {
        case 0: token->type = TOKEN_TYPE_END; break;
        case '(': token->type = TOKEN_TYPE_LPAREN; break;
        case ')': token->type = TOKEN_TYPE_RPAREN; break;
        case '{': token->type = TOKEN_TYPE_LCURLY; break;
        case '}': token->type = TOKEN_TYPE_RCURLY; break;
        case '[': token->type = TOKEN_TYPE_LSQUARE; break;
        case ']': token->type = TOKEN_TYPE_RSQUARE; break;
        case '+': token->type = TOKEN_TYPE_PLUS; break;
        case '-': token->type = TOKEN_TYPE_MINUS; break;
        case '*': token->type = TOKEN_TYPE_ASTERISK; break;
        case '/': token->type = TOKEN_TYPE_SLASH; break;
        case '%': token->type = TOKEN_TYPE_MODULUS; break;
        case '&': token->type = TOKEN_TYPE_AMPERSAND; break;
        case '|': token->type = TOKEN_TYPE_PIPE; break;
        case '~': token->type = TOKEN_TYPE_TILDE; break;
        case '^': token->type = TOKEN_TYPE_CIRCUMFLEX; break;
        case ',': token->type = TOKEN_TYPE_COMMA; break;
        case ':': token->type = TOKEN_TYPE_COLON; break;
        case '=': token->type = TOKEN_TYPE_EQUALS; break;
        case '$': token->type = TOKEN_TYPE_DOLLAR; break;
        case '.': token->type = TOKEN_TYPE_DOT; break;
        case '\n': token->type = TOKEN_TYPE_NEWLINE; lexer->current_line++; break;
        case '<':
        {
            token->size++;
            lexer->buffer_at++;
            if (lexer->buffer_at[0] == '<')
            {
                token->type = TOKEN_TYPE_LEFT_SHIFT;
            }
            break;
        }
        case '>':
        {
            token->size++;
            lexer->buffer_at++;
            if (lexer->buffer_at[0] == '>')
            {
                token->type = TOKEN_TYPE_RIGHT_SHIFT;
            }
            break;
        }
        case '"':
        {
            int copy_ahead = 0;
            BOOL escape = FALSE;

            // String literals
            do
            {
                escape = FALSE;

                token->size++;
                lexer->buffer_at++;

                if (!rewind)
                {
                    if (copy_ahead > 0)
                    {
                        lexer->buffer_at[0] = lexer->buffer_at[copy_ahead];
                    }                    

                    // Escape characters
                    if (lexer->buffer_at[0] == '\\')
                    {
                        char escaped_character = 0;
                        if (!get_escaped_character(lexer->buffer_at[1 + copy_ahead], &escaped_character))
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Invalid escape character '%c'", lexer->buffer_at[1 + copy_ahead]);
                            return 1;
                        }
                        lexer->buffer_at[0] = escaped_character;
                        copy_ahead++;                        
                        escape = TRUE;
                    }
                }

                if (!escape && (lexer->buffer_at[0] == '\n' || lexer->buffer_at[0] == '\0'))
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Unexpected end in string literal", "");
                    return 1;
                }
            } while (escape || lexer->buffer_at[0] != '"');

            token->type = TOKEN_TYPE_STRING;
            
            token->value++;
            token->size -= 2;
            if (!rewind)
            {
                token->value[token->size] = '\0'; // zero terminate value in string literal tokens
            }

            lexer->buffer_at += copy_ahead;

            break;
        }
        case '\'':
        {
            token->size = 1;
            lexer->buffer_at++;

            if (lexer->buffer_at[0] == '\'')
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Unexpected character \"%c\" inside character literal", lexer->buffer_at[0]);
                return 1;
            }

            if (lexer->buffer_at[0] == '\\')
            {
                lexer->buffer_at++;
                char escaped_character = '\0';
                if (!get_escaped_character(lexer->buffer_at[0], &escaped_character))
                {
                    write_compiler_error(lexer->filename, lexer->current_line, "Unexpected character \"%c\" inside character literal", lexer->buffer_at[0]);
                    return 1;
                }

                if (!rewind)
                {
                    lexer->buffer_at[0] = escaped_character;
                }
            }

            token->type = TOKEN_TYPE_CHARACTER;
            token->value = lexer->buffer_at;
            token->number_value = (int64_t) lexer->buffer_at[0];

            lexer->buffer_at++;
            if (lexer->buffer_at[0] != '\'')
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Unexpected character \"%c\" inside character literal", lexer->buffer_at[0]);
                return 1;
            }

            break;
        }
        case '#':
        {
            // Identifiers with the # prefix
            token->size = 0;
            do
            {
                token->size++;
                lexer->buffer_at++;
            } while (is_alpha(lexer->buffer_at[0]) || is_digit(lexer->buffer_at[0]) || lexer->buffer_at[0] == '_');
            if (is_str_equal_token_value(token, "#include"))
            {
                token->type = TOKEN_TYPE_INCLUDE;
            }
            else if (is_str_equal_token_value(token, "#print"))
            {
                token->type = TOKEN_TYPE_PRINT;
            }
            else if (is_str_equal_token_value(token, "#origin"))
            {
                token->type = TOKEN_TYPE_ORIGIN;
            }
            else if (is_str_equal_token_value(token, "#output_on"))
            {
                token->type = TOKEN_TYPE_OUTPUT_ON;
            }
            else if (is_str_equal_token_value(token, "#output_off"))
            {
                token->type = TOKEN_TYPE_OUTPUT_OFF;
            }
            else if (is_str_equal_token_value(token, "#include_binary"))
            {
                token->type = TOKEN_TYPE_INCLUDE_BINARY;
            }
            else if (is_str_equal_token_value(token, "#output_file"))
            {
                token->type = TOKEN_TYPE_SET_OUTPUT_FILE;
            }
            else if (is_str_equal_token_value(token, "#cpu_type"))
            {
                token->type = TOKEN_TYPE_SET_CPU_TYPE;
            }
            else if (is_str_equal_token_value(token, "#ifdef"))
            {
                token->type = TOKEN_TYPE_IFDEF;
            }
            else if (is_str_equal_token_value(token, "#ifndef"))
            {
                token->type = TOKEN_TYPE_IFNDEF;
            }
            else if (is_str_equal_token_value(token, "#else"))
            {
                token->type = TOKEN_TYPE_IFDEF_ELSE;
            }
            else if (is_str_equal_token_value(token, "#endif"))
            {
                token->type = TOKEN_TYPE_IFDEF_ENDIF;
            }
            else if (is_str_equal_token_value(token, "#define"))
            {
                token->type = TOKEN_TYPE_DEFINE;
            }
            else
            {
                write_compiler_error(lexer->filename, lexer->current_line, "Unexpected token \"%.*s\"", token->size, token->value);
                return 1;
            }
            lexer->buffer_at--;
            break;
        }
        default:
            if (is_alpha(lexer->buffer_at[0]) || lexer->buffer_at[0] == '_')
            {
                token->size = 0;
                do
                {
                    token->size++;
                    lexer->buffer_at++;
                } while (is_alpha(lexer->buffer_at[0]) || is_digit(lexer->buffer_at[0]) || lexer->buffer_at[0] == '_');

                if(lexer->buffer_at[0] == ':')
                {
                    if(lexer->buffer_at[1] == ':')
                    {
                        lexer->buffer_at+=2;
                        token->value2 = lexer->buffer_at;
                        token->size2 = 0;
                        if (is_alpha(lexer->buffer_at[0]) || lexer->buffer_at[0] == '_')
                        {
                            do
                            {
                                token->size2++;
                                lexer->buffer_at++;
                            } while (is_alpha(lexer->buffer_at[0]) || is_digit(lexer->buffer_at[0]) || lexer->buffer_at[0] == '_');
                            lexer->buffer_at--;
                            token->type = TOKEN_TYPE_IDENTIFIER_OF_LIBRARY;
                            break;
                        } 
                        else
                        {
                            write_compiler_error(lexer->filename, lexer->current_line, "Expected a character or \"_\", found token \"%.*s\"", token->size, token->value);
                        }
                    }
                    else
                    {
                        token->type = TOKEN_TYPE_LABEL;
                        break;
                    }
                }
                else if (lexer->buffer_at[0] == '\'')
                {
                    // af'
                    if (token->size == 2 && token->value[0] == 'a' && token->value[1] == 'f')
                    {
                        token->type = TOKEN_TYPE_REGISTER;
                        token->size++;
                        break;
                    }
                }
                else
                {
                    if (is_token_in_list(token, register_names, NUM_REGISTERS))
                    {
                        token->type = TOKEN_TYPE_REGISTER;
                    }
                    else if (is_token_in_list(token, op_names, NUM_OPS))
                    {
                        token->type = TOKEN_TYPE_OP;
                    }
                    else if (is_token_in_list(token, cond_names, NUM_CONDS))
                    {
                        token->type = TOKEN_TYPE_COND;
                    }
                    else if (is_str_equal_token_value(token, "function"))
                    {
                        token->type = TOKEN_TYPE_FUNCTION;
                    }
                    else if (is_str_equal_token_value(token, "interrupt"))
                    {
                        token->type = TOKEN_TYPE_INTERRUPT;
                    }
                    else if (is_str_equal_token_value(token, "library"))
                    {
                        token->type = TOKEN_TYPE_LIBRARY;
                    }
                    else if (is_str_equal_token_value(token, "data"))
                    {
                        token->type = TOKEN_TYPE_DATA;
                    }
                    else if (is_str_equal_token_value(token, "const"))
                    {
                        token->type = TOKEN_TYPE_CONST;
                    }
                    else if (is_str_equal_token_value(token, "struct"))
                    {
                        token->type = TOKEN_TYPE_STRUCT;
                    }
                    else if (is_str_equal_token_value(token, "union"))
                    {
                        token->type = TOKEN_TYPE_UNION;
                    }
                    else if (is_str_equal_token_value(token, "inline"))
                    {
                        token->type = TOKEN_TYPE_INLINE;
                    }
                    else if (is_str_equal_token_value(token, "if"))
                    {
                        token->type = TOKEN_TYPE_IF;
                    }
                    else if (is_str_equal_token_value(token, "else"))
                    {
                        token->type = TOKEN_TYPE_ELSE;
                    }
                    else if (is_str_equal_token_value(token, "while"))
                    {
                        token->type = TOKEN_TYPE_WHILE;
                    }
                    else if (is_str_equal_token_value(token, "do"))
                    {
                        token->type = TOKEN_TYPE_DO;
                    }
                    else if (is_str_equal_token_value(token, "forever"))
                    {
                        token->type = TOKEN_TYPE_FOREVER;
                    }
                    else if (is_str_equal_token_value(token, "break"))
                    {
                        token->type = TOKEN_TYPE_BREAK;
                    }
                    else if (is_str_equal_token_value(token, "sizeof"))
                    {
                        token->type = TOKEN_TYPE_SIZEOF;
                    }
                    else if (is_str_equal_token_value(token, "length"))
                    {
                        token->type = TOKEN_TYPE_LENGTH;
                    }
                    else if (is_str_equal_token_value(token, "from"))
                    {
                        token->type = TOKEN_TYPE_FROM;
                    }
                    else if (is_str_equal_token_value(token, "byte") || is_str_equal_token_value(token, "word") || is_str_equal_token_value(token, "dword"))
                    {
                        token->type = TOKEN_TYPE_DATA_TYPE;
                    }
                    else
                    {
                        token->type = TOKEN_TYPE_IDENTIFIER;
                    }
                    lexer->buffer_at--;
                    break;
                }
            }

            if (is_digit(lexer->buffer_at[0]))
            {
                // Numbers
                int base = 10;
                if (lexer->buffer_at[0] == '0')
                {
                    if (lexer->buffer_at[1] == 'b')
                    {
                        base = 2;
                        lexer->buffer_at += 2;
                    }
                    else if (lexer->buffer_at[1] == 'o')
                    {
                        base = 8;
                        lexer->buffer_at += 2;
                    }
                    else if (lexer->buffer_at[1] == 'x')
                    {
                        base = 16;
                        lexer->buffer_at += 2;
                    }
                }
                token->value = lexer->buffer_at;

                token->size = 0;
                do
                {
                    token->size++;
                    lexer->buffer_at++;                
                } while (
                    (base == 10 && lexer->buffer_at[0] >= '0' && lexer->buffer_at[0] <= '9') ||
                    (base == 2 && lexer->buffer_at[0] >= '0' && lexer->buffer_at[0] <= '1') ||
                    (base == 8 && lexer->buffer_at[0] >= '0' && lexer->buffer_at[0] <= '7') ||
                    (base == 16 && ((lexer->buffer_at[0] >= '0' && lexer->buffer_at[0] <= '9') || 
                                    (lexer->buffer_at[0] >= 'a' && lexer->buffer_at[0] <= 'f') ||
                                    (lexer->buffer_at[0] >= 'A' && lexer->buffer_at[0] <= 'F'))));

                token->type = TOKEN_TYPE_NUMBER;
                token->number_value = str_to_number(token->value, token->size, base);
                lexer->buffer_at--;
                break;
            }

            // write_compiler_error(lexer->filename, lexer->current_line, "Unexpected character \"%c\"", lexer->buffer_at[0]);
            // return 1;

            token->type = TOKEN_TYPE_INVALID;
    }

    lexer->buffer_at++;

    if (rewind)
    {
        lexer->buffer_at = buffer_previous_position;
        lexer->current_line = previous_line;
    }

    return 0;
}

int get_next_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline)
{
    return get_token(lexer, token, skip_newline, FALSE, TRUE);
}

int peek_next_token(struct Lexer *lexer, struct Token *token, BOOL skip_newline)
{
    return get_token(lexer, token, skip_newline, TRUE, TRUE);
}
