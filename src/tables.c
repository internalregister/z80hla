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

struct ConstantList
{
	char *name;
	int name_size;

	char *library_name;
	int library_size;

	int64_t value;

	struct ConstantList *next;
};

struct ConstantList *first_constant = NULL;

void fprint_constants(FILE *fp)
{
	struct ConstantList *current_constant = first_constant;
	BOOL first_constant = TRUE;

	fprintf(fp, "{\n");

	while(current_constant != NULL)
	{
		if (!first_constant)
		{
			fprintf(fp, ",\n");
		}
		if (current_constant->library_size == 0)
		{
			fprintf(fp, "\t\"%.*s\":%"PRId64"", current_constant->name_size, current_constant->name, current_constant->value);
		}
		else
		{
			fprintf(fp, "\t\"%.*s::%.*s\":%"PRId64"", current_constant->library_size, current_constant->library_name, current_constant->name_size, current_constant->name, current_constant->value);
		}
		first_constant = FALSE;
		current_constant = current_constant->next;
	}

	fprintf(fp, "}");
}

void fprintf_output_symbols(FILE *fp)
{
	struct ConstantList *current_constant = first_constant;
	BOOL first_constant = TRUE;

	fprintf(fp, "{\n");

	while(current_constant != NULL)
	{
		if (!first_constant)
		{
			fprintf(fp, ",\n");
		}
		if (current_constant->library_size == 0)
		{
			fprintf(fp, "\t\"%.*s\":%"PRId64"", current_constant->name_size, current_constant->name, current_constant->value);
		}
		else
		{
			fprintf(fp, "\t\"%.*s::%.*s\":%"PRId64"", current_constant->library_size, current_constant->library_name, current_constant->name_size, current_constant->name, current_constant->value);
		}
		first_constant = FALSE;
		current_constant = current_constant->next;
	}

	fprintf(fp, "}");
}

int set_constant(char *library_name, int library_size, char *name, int name_size, int64_t value)
{
	struct ConstantList *current_constant = first_constant, *prev_constant = NULL;	
	struct ConstantList *new_constant = NULL;

	while(current_constant != NULL)
	{
		// Search for the constant
		if (is_str_equal2(current_constant->name, current_constant->name_size, name, name_size) &&
			is_str_equal2(current_constant->library_name, current_constant->library_size, library_name, library_size))
		{
			current_constant->value = value;
			return 1;
		}

		prev_constant = current_constant;
		current_constant = current_constant->next;
	}

	new_constant = (struct ConstantList*)malloc(sizeof(struct ConstantList));
	new_constant->name = name;
	new_constant->name_size = name_size;
	new_constant->library_name = library_name;
	new_constant->library_size = library_size;
	new_constant->value = value;
	new_constant->next = NULL;

	if (prev_constant == NULL)
	{
		first_constant = new_constant;
	}
	else
	{
		prev_constant->next = new_constant;
	}

	return 0;
}

int get_constant(char *library_name, int library_size, char *name, int name_size, int64_t *value)
{
	struct ConstantList *current_constant = first_constant;

	while(current_constant != NULL)
	{
		// Search for the constant
		if (is_str_equal2(current_constant->name, current_constant->name_size, name, name_size) &&
			is_str_equal2(current_constant->library_name, current_constant->library_size, library_name, library_size))
		{
			*value = current_constant->value;
			return 0;
		}

		current_constant = current_constant->next;
	}

	return 1;
}

BOOL is_constant_present(char *library_name, int library_size, char *name, int name_size)
{
	int64_t value;
	return (get_constant(library_name, library_size, name, name_size, &value) == 0);
}

// *************
// Include stack
// *************

struct IncludeFileStack
{
	char *filename;

	struct IncludeFileStack *next_element, *last_element;
};

struct IncludeFileStack *start_include_stack = NULL, *end_include_stack = NULL;
int include_stack_size = 0;

int push_include_file(char *filename)
{
	struct IncludeFileStack *element;	

	if (include_stack_size >= INCLUDE_STACK_MAX)
	{
		return INCLUDE_ERROR_OVER_MAX_STACK;
	}

	// Look for this filename in the stack
	element = start_include_stack;
	while(element != NULL)
	{
		if (!strcmp(element->filename, filename))
		{
			return INCLUDE_ERROR_CYCLIC;
		}
		element = element->next_element;
	}

	element = (struct IncludeFileStack*)malloc(sizeof(*element));
	element->filename = filename;
	element->next_element = NULL;

	if (end_include_stack == NULL)
	{
		start_include_stack = end_include_stack = element;
		start_include_stack->last_element = NULL;
	}
	else
	{
		element->last_element = end_include_stack;
		end_include_stack->next_element = element;
		end_include_stack = element;		
	}

	include_stack_size++;

	write_debug("Pushed include file \"%s\" to the stack", filename);

	return 0;
}

void pop_include_file()
{
	struct IncludeFileStack *element;
	element = end_include_stack;
	if (end_include_stack != NULL)
	{
		if (end_include_stack->last_element != NULL)
		{
			end_include_stack->last_element->next_element = NULL;
			end_include_stack = end_include_stack->last_element;
		}
		else
		{
			start_include_stack = end_include_stack = NULL;
		}

		write_debug("Popped include file \"%s\" to the stack", element->filename);

		free(element);

		include_stack_size--;		
	}
}

// *************
// Source cache
// *************

struct ContentCache
{
	char *filename;
	char *content;

	struct ContentCache *next_element;
};

struct ContentCache *first_content_cache_element = NULL, *last_content_cache_element = NULL;

char *get_content_from_cache(char *filename)
{
	struct ContentCache *current_element = first_content_cache_element;	

	while(current_element != NULL)
	{
		if (!strcmp(current_element->filename, filename))
		{
			return current_element->content;
		}
		current_element = current_element->next_element;
	}

	return NULL;
}

void add_content_to_cache(char *filename, char *content)
{
	struct ContentCache *new_element;

	new_element = (struct ContentCache*)malloc(sizeof(struct ContentCache));
	new_element->filename = filename;
	new_element->content = content;
	new_element->next_element = NULL;

	if (last_content_cache_element == NULL)
	{
		first_content_cache_element = last_content_cache_element = new_element;
	}
	else
	{
		last_content_cache_element->next_element = new_element;
		last_content_cache_element = new_element;
	}
}

// *************
// Libraries
// *************

struct SymbolList
{
	char *library_name;
	int library_size;

	char *symbol_name;
	int symbol_name_size;

	struct SymbolList *next;
};

struct SymbolDependencies
{
	char *library_name;
	int library_size;

	char *symbol_name;
	int symbol_name_size;

	struct SymbolList *dependencies;

	struct SymbolDependencies *next;
};

static struct SymbolDependencies *library_symbol_dependencies = NULL;
static struct SymbolList *library_symbols_used = NULL;

void add_library_symbol_dependency(char *library_name, int library_size, char *symbol_name, int symbol_name_size,
	char *library_dependency_name, int library_dependency_name_size, char *symbol_dependency_name, int symbol_dependency_name_size)
{
	struct SymbolDependencies *current_symbol = library_symbol_dependencies, *last_symbol = library_symbol_dependencies;

	while(current_symbol != NULL)
	{
		if (is_str_equal2(library_name, library_size, current_symbol->library_name, current_symbol->library_size) && 
		    is_str_equal2(symbol_name, symbol_name_size, current_symbol->symbol_name, current_symbol->symbol_name_size))
		{
			struct SymbolList *current_dependency = current_symbol->dependencies, *last_dependency = current_symbol->dependencies;

			while(current_dependency != NULL)
			{
				if (is_str_equal2(library_dependency_name, library_dependency_name_size, current_dependency->library_name, current_dependency->library_size) && 
		    		is_str_equal2(symbol_dependency_name, symbol_dependency_name_size, current_dependency->symbol_name, current_dependency->symbol_name_size))
				{
					// Dependency already here
					return;
				}

				last_dependency = current_dependency;
				current_dependency = current_dependency->next;
			}

			struct SymbolList *new_symbol = (struct SymbolList *)malloc(sizeof(struct SymbolList));
			new_symbol->library_name = library_dependency_name;
			new_symbol->library_size = library_dependency_name_size;
			new_symbol->symbol_name = symbol_dependency_name;
			new_symbol->symbol_name_size = symbol_dependency_name_size;
			new_symbol->next = NULL;

			if (current_symbol->dependencies == NULL)
			{
				current_symbol->dependencies = new_symbol;
			}
			else
			{
				last_dependency->next = new_symbol;
			}

			return;
		}

		last_symbol = current_symbol;
		current_symbol = current_symbol->next;
	}

	struct SymbolDependencies *new_dependency = (struct SymbolDependencies *)malloc(sizeof(struct SymbolDependencies));
	new_dependency->library_name = library_name;
	new_dependency->library_size = library_size;
	new_dependency->symbol_name = symbol_name;
	new_dependency->symbol_name_size = symbol_name_size;
	new_dependency->next = NULL;

	struct SymbolList *new_symbol = (struct SymbolList *)malloc(sizeof(struct SymbolList));
	new_symbol->library_name = library_dependency_name;
	new_symbol->library_size = library_dependency_name_size;
	new_symbol->symbol_name = symbol_dependency_name;
	new_symbol->symbol_name_size = symbol_dependency_name_size;
	new_symbol->next = NULL;

	new_dependency->dependencies = new_symbol;
	
	if (library_symbol_dependencies == NULL)
	{
		library_symbol_dependencies = new_dependency;
	}
	else
	{
		last_symbol->next = new_dependency;
	}
}

void fprint_library_symbol_dependencies(FILE *fp)
{
	struct SymbolDependencies *current_symbol = library_symbol_dependencies;
	BOOL first_symbol = TRUE, first_dependency = TRUE;

	fprintf(fp, "[");

	while(current_symbol != NULL)
	{
		struct SymbolList *current_dependency = current_symbol->dependencies;

		if (!first_symbol)
		{
			fprintf(fp, ",");
		}

		fprintf(fp, "{\"library_name\": \"%.*s\",", current_symbol->library_size, current_symbol->library_name);
		fprintf(fp, "\"symbol_name\": \"%.*s\",", current_symbol->symbol_name_size, current_symbol->symbol_name);
		
		fprintf(fp, "\"dependencies\": [");
		first_dependency = TRUE;
		while(current_dependency != NULL)
		{
			if (!first_dependency)
			{
				fprintf(fp, ",");
			}

			fprintf(fp, "{\"library_name\": \"%.*s\",", current_dependency->library_size, current_dependency->library_name);
			fprintf(fp, "\"symbol_name\": \"%.*s\"}", current_dependency->symbol_name_size, current_dependency->symbol_name);

			current_dependency = current_dependency->next;
			first_dependency = FALSE;
		}
		fprintf(fp, "]}");

		current_symbol = current_symbol->next;
		first_symbol = FALSE;
	}

	fprintf(fp, "]");
}

void add_library_symbol_used(char *library_name, int library_size, char *symbol_name, int symbol_name_size)
{
	struct SymbolList *current_symbol = library_symbols_used, *last_symbol = library_symbols_used;

	while(current_symbol != NULL)
	{
		if (is_str_equal2(library_name, library_size, current_symbol->library_name, current_symbol->library_size) && 
		    is_str_equal2(symbol_name, symbol_name_size, current_symbol->symbol_name, current_symbol->symbol_name_size))
		{
			return;
		}

		last_symbol = current_symbol;
		current_symbol = current_symbol->next;
	}

	struct SymbolList *new_symbol = (struct SymbolList *)malloc(sizeof(struct SymbolList));
	new_symbol->library_name = library_name;
	new_symbol->library_size = library_size;
	new_symbol->symbol_name = symbol_name;
	new_symbol->symbol_name_size = symbol_name_size;
	new_symbol->next = NULL;

	if (last_symbol == NULL)
	{
		library_symbols_used = new_symbol;
	}
	else
	{
		last_symbol->next = new_symbol;
	}
}

void fprint_library_symbols_used(FILE *fp)
{
	struct SymbolList *current_symbol = library_symbols_used;
	BOOL first_symbol = TRUE;

	fprintf(fp, "[");

	while(current_symbol != NULL)
	{
		if (!first_symbol)
		{
			fprintf(fp, ",");
		}

		fprintf(fp, "{\"library\": \"%.*s\",", current_symbol->library_size, current_symbol->library_name);
		fprintf(fp, "\"symbol\": \"%.*s\"}", current_symbol->symbol_name_size, current_symbol->symbol_name);

		first_symbol = FALSE;
		current_symbol = current_symbol->next;
	}

	fprintf(fp, "]");
}

void fill_library_symbols_used_with_dependencies()
{
	struct SymbolList *current_symbol = library_symbols_used;

	while(current_symbol != NULL)
	{
		struct SymbolDependencies *current_symbol2 = library_symbol_dependencies;

		while(current_symbol2 != NULL)
		{
			if (is_str_equal2(current_symbol->library_name, current_symbol->library_size, current_symbol2->library_name, current_symbol2->library_size) && 
				is_str_equal2(current_symbol->symbol_name, current_symbol->symbol_name_size, current_symbol2->symbol_name, current_symbol2->symbol_name_size))
			{
				struct SymbolList *current_dependency = current_symbol2->dependencies;

				while(current_dependency != NULL)
				{
					add_library_symbol_used(
						current_dependency->library_name, current_dependency->library_size,
						current_dependency->symbol_name, current_dependency->symbol_name_size);

					current_dependency = current_dependency->next;					
				}

				break;
			}
			current_symbol2 = current_symbol2->next;
		}

		current_symbol = current_symbol->next;
	}
}

BOOL is_library_symbol_needed(char *library_name, int library_size, char *symbol_name, int symbol_name_size)
{
	struct SymbolList *current_symbol = library_symbols_used;

	if (assemble_all)
	{
		return TRUE;
	}

	while(current_symbol != NULL)
	{
		if (is_str_equal2(library_name, library_size, current_symbol->library_name, current_symbol->library_size) && 
		    is_str_equal2(symbol_name, symbol_name_size, current_symbol->symbol_name, current_symbol->symbol_name_size))
		{
			return TRUE;
		}

		current_symbol = current_symbol->next;
	}

	return FALSE;
}

// *************************
//  Structs and Unions
// *************************

struct StructuredType *first_structured_type = NULL;

struct StructuredType *create_structured_type(char *name, int name_size, char *library_name, int library_name_size, enum StructuredTypeType type)
{
	struct StructuredType *current_type = first_structured_type;

	while(current_type != NULL)
	{
		if (is_str_equal2(name, name_size, current_type->name, current_type->name_size) &&
		    is_str_equal2(library_name, library_name_size, current_type->library_name, current_type->library_name_size))
		{
			return NULL;
		}

		current_type = current_type->next;
	}

	struct StructuredType *new_type = (struct StructuredType *)malloc(sizeof(struct StructuredType));
	new_type->name = name;
	new_type->name_size = name_size;
	new_type->library_name = library_name;
	new_type->library_name_size = library_name_size;
	new_type->struct_size = 0;
	new_type->type = type;
	new_type->first_element = NULL;
	new_type->next = NULL;

	return new_type;
}

struct StructuredType *add_structured_type(struct StructuredType *new_type)
{
	struct StructuredType *current_type = first_structured_type, *last_type = NULL;

	while(current_type != NULL)
	{
		if (is_str_equal2(new_type->name, new_type->name_size, current_type->name, current_type->name_size) &&
		    is_str_equal2(new_type->library_name, new_type->library_name_size, current_type->library_name, current_type->library_name_size))
		{
			return NULL;
		}

		last_type = current_type;
		current_type = current_type->next;
	}

	if (last_type == NULL)
	{
		first_structured_type = new_type;
	}
	else
	{
		last_type->next = new_type;
	}

	return new_type;
}

struct StructuredType *get_structured_type(char *name, int name_size, char *library_name, int library_name_size)
{
	struct StructuredType *current_type = first_structured_type;

	while(current_type != NULL)
	{
		if (is_str_equal2(name, name_size, current_type->name, current_type->name_size) &&
		    is_str_equal2(library_name, library_name_size, current_type->library_name, current_type->library_name_size))
		{
			return current_type;
		}

		current_type = current_type->next;
	}

	return NULL;
}

int add_element_to_structured_type(struct StructuredType *structured_type, char *name, int name_size, char *type, int type_size, char *type_library, int type_library_size, int array_length)
{
	struct StructElement *current_element, *last_element = NULL;
	current_element = structured_type->first_element;

	while(current_element != NULL)
	{
		if (is_str_equal2(name, name_size, current_element->name, current_element->name_size))
		{
			return 1;
		}

		last_element = current_element;
		current_element = current_element->next;
	}

	struct StructElement *new_element = (struct StructElement *)malloc(sizeof(struct StructElement));
	new_element->name = name;
	new_element->name_size = name_size;
	new_element->type = type;
	new_element->type_size = type_size;
	new_element->type_library = type_library;
	new_element->type_library_size = type_library_size;
	new_element->is_native_type = TRUE;
	new_element->structured_type = NULL;
	new_element->array_length = array_length;
	new_element->position = structured_type->type == STRUCT_TYPE_STRUCT ? structured_type->struct_size : 0;
	new_element->next = NULL;

	if (last_element == NULL)
	{
		structured_type->first_element = new_element;
	}
	else
	{
		last_element->next = new_element;
	}

	if (is_str_equal(new_element->type, new_element->type_size, "byte"))
	{
		if (structured_type->type == STRUCT_TYPE_STRUCT)
		{
			structured_type->struct_size += 1 * new_element->array_length;
		}
		else
		{
			structured_type->struct_size = MAX(structured_type->struct_size, 1 * new_element->array_length);
		}
		new_element->size_of_type = 1;
	}
	else if (is_str_equal(new_element->type, new_element->type_size, "word"))
	{
		if (structured_type->type == STRUCT_TYPE_STRUCT)
		{
			structured_type->struct_size += 2 * new_element->array_length;
		}
		else
		{
			structured_type->struct_size = MAX(structured_type->struct_size, 2 * new_element->array_length);
		}
		new_element->size_of_type = 2;
	}
	else if (is_str_equal(new_element->type, new_element->type_size, "dword"))
	{
		if (structured_type->type == STRUCT_TYPE_STRUCT)
		{
			structured_type->struct_size += 4 * new_element->array_length;
		}
		else
		{
			structured_type->struct_size = MAX(structured_type->struct_size, 4 * new_element->array_length);
		}
		new_element->size_of_type = 4;
	}
	else
	{
		struct StructuredType *inner_structured_type = get_structured_type(new_element->type, new_element->type_size, new_element->type_library, new_element->type_library_size);
		if (inner_structured_type == NULL) { return 1; }

		if (structured_type->type == STRUCT_TYPE_STRUCT)
		{
			structured_type->struct_size += inner_structured_type->struct_size * new_element->array_length;
		}
		else
		{
			structured_type->struct_size = MAX(structured_type->struct_size, inner_structured_type->struct_size * new_element->array_length);
		}

		new_element->is_native_type = FALSE;
		new_element->structured_type = inner_structured_type;
		new_element->size_of_type = inner_structured_type->struct_size;
	}

	return 0;
}

void fprint_structured_types(FILE *fp)
{
	struct StructuredType *current_structured_type = first_structured_type;
	BOOL is_first_structured_type = TRUE;

	fprintf(fp, "[");

	while (current_structured_type != NULL)
	{
		if (!is_first_structured_type)
		{
			fprintf(fp, ",");
		}

		fprintf(fp, "{\"name\": \"%.*s\",", current_structured_type->name_size, current_structured_type->name);
		fprintf(fp, "\"library\": \"%.*s\",", current_structured_type->library_name_size, current_structured_type->library_name);
		fprintf(fp, "\"type\": \"%s\",", current_structured_type->type == STRUCT_TYPE_STRUCT ? "struct" : "union");
		fprintf(fp, "\"size\": \"%d\",", current_structured_type->struct_size);
		fprintf(fp, "\"elements\": [");
		struct StructElement *current_element = current_structured_type->first_element;
		BOOL is_first_element = TRUE;
		while (current_element != NULL)
		{
			if (!is_first_element)
			{
				fprintf(fp, ",");
			}
			fprintf(fp, "{");
			fprintf(fp, "\"name\": \"%.*s\",", current_element->name_size, current_element->name);
			fprintf(fp, "\"type\": \"%.*s\",", current_element->type_size, current_element->type);
			fprintf(fp, "\"array_length\": \"%d\",", current_element->array_length);
			fprintf(fp, "\"position\": \"%d\",", current_element->position);
			fprintf(fp, "\"size\": \"%d\"", current_element->size_of_type);
			fprintf(fp, "}");
			is_first_element = FALSE;
			current_element = current_element->next;
		}
		fprintf(fp, "]}");

		is_first_structured_type = FALSE;
		current_structured_type = current_structured_type->next;
	}

	fprintf(fp, "]");
}

struct StructElement *get_struct_element(struct StructuredType *structured_type, char *name, int name_size)
{
	struct StructElement *struct_element = structured_type->first_element;

	while(struct_element != NULL)
	{
		if (is_str_equal2(struct_element->name, struct_element->name_size, name, name_size))
		{
			return struct_element;
		}

		struct_element = struct_element->next;
	}

	return NULL;
}

struct ASTNode **struct_bytes = NULL;

void clear_struct_bytes(int size)
{
	if (struct_bytes == NULL)
	{
		struct_bytes = (struct ASTNode **)malloc(sizeof(struct ASTNode *) * 65536);
	}

	assert(size > 0 && size <= 65536);

	memset(struct_bytes, 0, size * sizeof(struct ASTNode *));
}

int set_struct_bytes(struct StructuredType *structured_type, int offset, struct StructElement *struct_element, int index, struct ASTNode *node)
{
	assert(struct_bytes != NULL);
	
	if (is_str_equal(struct_element->type, struct_element->type_size, "byte") && is_str_equal(struct_element->type_library, struct_element->type_library_size, ""))
	{
		node->type = NODE_TYPE_EXPRESSION_8;
		struct_bytes[offset + struct_element->position + index] = node;
	}
	else if (is_str_equal(struct_element->type, struct_element->type_size, "word") && is_str_equal(struct_element->type_library, struct_element->type_library_size, ""))
	{
		node->type = NODE_TYPE_EXPRESSION_16;
		struct_bytes[offset + struct_element->position + index * 2] = node;
		struct_bytes[offset + struct_element->position + index * 2 + 1] = NULL;
	}
	else if (is_str_equal(struct_element->type, struct_element->type_size, "dword") && is_str_equal(struct_element->type_library, struct_element->type_library_size, ""))
	{
		node->type = NODE_TYPE_EXPRESSION_32;
		struct_bytes[offset + struct_element->position + index * 4] = node;
		struct_bytes[offset + struct_element->position + index * 4 + 1] = NULL;
		struct_bytes[offset + struct_element->position + index * 4 + 2] = NULL;
		struct_bytes[offset + struct_element->position + index * 4 + 3] = NULL;
	}
	else
	{
		return 1;
	}

	return 0;
}

struct DataSymbol *first_data_symbol = NULL, *last_data_symbol = NULL;

int add_data_symbol(char *name, int name_size, char *library_name, int library_name_size, char *type, int type_size, char *library_type, int library_type_size, int length)
{
	struct DataSymbol *current_data_symbol = first_data_symbol, *last_current_data_symbol = NULL;

	while(current_data_symbol != NULL)
	{
		if (is_str_equal2(name, name_size, current_data_symbol->name, current_data_symbol->name_size) &&
			is_str_equal2(library_name, library_name_size, current_data_symbol->library_name, current_data_symbol->library_name_size))
		{
			return 1;
		}

		last_current_data_symbol = current_data_symbol;
		current_data_symbol = current_data_symbol->next;
	}

	struct DataSymbol *new_data_symbol = (struct DataSymbol *)malloc(sizeof(struct DataSymbol));

	new_data_symbol->name = name;
	new_data_symbol->name_size = name_size;
	new_data_symbol->library_name = library_name;
	new_data_symbol->library_name_size = library_name_size;
	new_data_symbol->type = type;
	new_data_symbol->type_size = type_size;
	new_data_symbol->library_type = library_type;
	new_data_symbol->library_type_size = library_type_size;

	if (is_str_equal(type, type_size, "byte") && is_str_equal(library_type, library_type_size, ""))
	{
		new_data_symbol->is_native_type = TRUE;
		new_data_symbol->size_of_type = 1;
		new_data_symbol->structured_type = NULL;
	}
	else if (is_str_equal(type, type_size, "word") && is_str_equal(library_type, library_type_size, ""))
	{
		new_data_symbol->is_native_type = TRUE;
		new_data_symbol->size_of_type = 2;
		new_data_symbol->structured_type = NULL;
	}
	else if (is_str_equal(type, type_size, "dword") && is_str_equal(library_type, library_type_size, ""))
	{
		new_data_symbol->is_native_type = TRUE;
		new_data_symbol->size_of_type = 4;
		new_data_symbol->structured_type = NULL;
	}
	else
	{
		new_data_symbol->is_native_type = FALSE;
		struct StructuredType *structured_type = get_structured_type(type, type_size, library_type, library_type_size);
		if (structured_type == NULL)
		{
			return 1;
		}		
		new_data_symbol->size_of_type = structured_type->struct_size;
		new_data_symbol->structured_type = structured_type;
	}

	new_data_symbol->length = length;

	new_data_symbol->next = NULL;

	if (last_current_data_symbol == NULL)
	{
		first_data_symbol = new_data_symbol;
	}
	else
	{
		last_current_data_symbol->next = new_data_symbol;
	}

	last_data_symbol = new_data_symbol;

	return 0;
}

struct DataSymbol *get_data_symbol(char *name, int name_size, char *library_name, int library_name_size)
{
	struct DataSymbol *current_data_symbol = first_data_symbol;

	while(current_data_symbol != NULL)
	{
		if (is_str_equal2(name, name_size, current_data_symbol->name, current_data_symbol->name_size) &&
			is_str_equal2(library_name, library_name_size, current_data_symbol->library_name, current_data_symbol->library_name_size))
		{
			return current_data_symbol;
		}

		current_data_symbol = current_data_symbol->next;
	}

	return NULL;
}

struct DataSymbol *get_last_data_symbol()
{
	return last_data_symbol;
}

void fprintf_data_symbols(FILE *fp)
{
	struct DataSymbol *current_data_symbol = first_data_symbol;
	fprintf(fp, "[");

	BOOL first = TRUE;

	while(current_data_symbol != NULL)
	{
		if (!first)
		{
			fprintf(fp, ",");
		}

		fprintf(fp, "{");
		if (current_data_symbol->library_name_size > 0)
		{
			fprintf(fp, "\"name\": \"%.*s::%.*s\",", current_data_symbol->library_name_size, current_data_symbol->library_name, current_data_symbol->name_size, current_data_symbol->name);
		}
		else
		{
			fprintf(fp, "\"name\": \"%.*s\",", current_data_symbol->name_size, current_data_symbol->name);
		}
		if (current_data_symbol->library_type_size > 0)
		{
			fprintf(fp, "\"type\": \"%.*s::%.*s\",", current_data_symbol->library_type_size, current_data_symbol->library_type, current_data_symbol->type_size, current_data_symbol->type);
		}
		else
		{
			fprintf(fp, "\"type\": \"%.*s\",", current_data_symbol->type_size, current_data_symbol->type);
		}
		fprintf(fp, "\"is_native_type\": %s,", current_data_symbol->is_native_type ? "true" : "false");
		fprintf(fp, "\"structured_type\": \"%p\",", current_data_symbol->structured_type);
		fprintf(fp, "\"size_of_type\": %d,", current_data_symbol->size_of_type);
		fprintf(fp, "\"length\": %d", current_data_symbol->length);
		fprintf(fp, "}");

		first = FALSE;
		current_data_symbol = current_data_symbol->next;
	}

	fprintf(fp, "]");
}

static struct InlineSymbol *first_inline_symbol = NULL;

struct InlineSymbol *add_inline_symbol(char *name, int name_size, char *library_name, int library_name_size, struct ASTNode *node)
{
	struct InlineSymbol *current_inline = first_inline_symbol, *last_inline = NULL;

	while(current_inline != NULL)
	{
		if (is_str_equal2(name, name_size, current_inline->name, current_inline->name_size) &&
		    is_str_equal2(library_name, library_name_size, current_inline->library_name, current_inline->library_name_size))
		{
			return NULL;
		}

		last_inline = current_inline;
		current_inline = current_inline->next;
	}

	struct InlineSymbol *new_inline = (struct InlineSymbol*)malloc(sizeof(struct InlineSymbol));
	new_inline->name = name;
	new_inline->name_size = name_size;
	new_inline->library_name = library_name;
	new_inline->library_name_size = library_name_size;
	new_inline->node = node;
	new_inline->argument_count = 0;
	new_inline->arguments = NULL;
	new_inline->next = NULL;

	if (last_inline == NULL)
	{
		first_inline_symbol = new_inline;
	}
	else
	{
		last_inline->next = new_inline;
	}

	return new_inline;
}

struct InlineSymbol *get_inline_symbol(char *name, int name_size, char *library_name, int library_name_size)
{
	struct InlineSymbol *current_inline = first_inline_symbol;

	while(current_inline != NULL)
	{
		if (is_str_equal2(name, name_size, current_inline->name, current_inline->name_size) &&
		    is_str_equal2(library_name, library_name_size, current_inline->library_name, current_inline->library_name_size))
		{
			return current_inline;
		}

		current_inline = current_inline->next;
	}

	return NULL;
}

int add_inline_symbol_argument(struct InlineSymbol *inline_symbol, char *name, int name_size)
{
	struct InlineArgument *current_argument = inline_symbol->arguments, *last_argument = NULL;

	while(current_argument != NULL)
	{
		if (is_str_equal2(name, name_size, current_argument->name, current_argument->name_size))
		{
			return 1;
		}

		last_argument = current_argument;
		current_argument = current_argument->next;
	}

	struct InlineArgument *new_argument = (struct InlineArgument *)malloc(sizeof(struct InlineArgument));
	new_argument->name = name;
	new_argument->name_size = name_size;
	new_argument->next = NULL;

	if (last_argument == NULL)
	{
		inline_symbol->arguments = new_argument;
	}
	else
	{
		last_argument->next = new_argument;
	}

	inline_symbol->argument_count++;

	return 0;
}

int get_inline_symbol_argument_index(struct InlineSymbol *inline_symbol, char *name, int name_size)
{	
	struct InlineArgument *current_argument = inline_symbol->arguments;
	int index = 0;	

	while(current_argument != NULL)
	{
		if (is_str_equal2(name, name_size, current_argument->name, current_argument->name_size))
		{
			return index;
		}

		current_argument = current_argument->next;
		index++;
	}

	return -1;
}

struct LoopLabel
{
	char *label_start;
	char *label_end;	

	struct LoopLabel *next, *prev;
};

static struct LoopLabel *first_loop_label = NULL, *last_loop_label = NULL;

void push_loop_label(char *label_start, char *label_end)
{
	struct LoopLabel *new_label = (struct LoopLabel*)malloc(sizeof(struct LoopLabel));
	new_label->label_start = label_start;
	new_label->label_end = label_end;
	new_label->next = NULL;
	new_label->prev = last_loop_label;

	if (first_loop_label == NULL)
	{
		first_loop_label = new_label;
	}
	else
	{
		first_loop_label->next = new_label;
	}

	last_loop_label = new_label;
}

char *peek_loop_label_start()
{
	if (last_loop_label == NULL)
	{
		return NULL;
	}

	return last_loop_label->label_start;
}

char *peek_loop_label_end()
{
	if (last_loop_label == NULL)
	{
		return NULL;
	}

	return last_loop_label->label_end;
}

void pop_loop_label()
{
	if (last_loop_label == NULL)
	{
		return;
	}

	struct LoopLabel *new_last_label = last_loop_label->prev;

	free(last_loop_label);

	if (last_loop_label == first_loop_label)
	{
		first_loop_label = last_loop_label = NULL;
		return;
	}

	new_last_label->next = NULL;
	last_loop_label = new_last_label;	
}

struct InlineSymbolStack
{
	char *name;
	int name_size;
	char *library_name;
	int library_name_size;

	struct InlineSymbolStack *next, *prev;
};

static struct InlineSymbolStack *first_inline_symbol_stack = NULL, *last_inline_symbol_stack = NULL;

void push_inline_symbol_stack(char *inline_symbol, int inline_symbol_size, char *library_name, int library_name_size)
{
	struct InlineSymbolStack *new_inline_symbol_stack = (struct InlineSymbolStack*)malloc(sizeof(struct InlineSymbolStack));
	new_inline_symbol_stack->library_name = library_name;
	new_inline_symbol_stack->library_name_size = library_name_size;
	new_inline_symbol_stack->name = inline_symbol;
	new_inline_symbol_stack->name_size = inline_symbol_size;
	new_inline_symbol_stack->next = NULL;
	new_inline_symbol_stack->prev = last_inline_symbol_stack;

	if (first_inline_symbol_stack == NULL)
	{
		first_inline_symbol_stack = new_inline_symbol_stack;
	}
	else
	{
		first_inline_symbol_stack->next = new_inline_symbol_stack;
	}

	last_inline_symbol_stack = new_inline_symbol_stack;
}

BOOL is_inline_symbol_in_stack(char *name, int name_size, char *library_name, int library_name_size)
{
	struct InlineSymbolStack *current_inline_symbol_stack = first_inline_symbol_stack;

	while (current_inline_symbol_stack != NULL)
	{
		if (is_str_equal2(current_inline_symbol_stack->name, current_inline_symbol_stack->name_size, name, name_size) &&
			is_str_equal2(current_inline_symbol_stack->library_name, current_inline_symbol_stack->library_name_size, library_name, library_name_size))
		{
			return TRUE;
		}

		current_inline_symbol_stack = current_inline_symbol_stack->next;
	}

	return FALSE;
}

void pop_inline_symbol_stack()
{
	if (last_inline_symbol_stack == NULL)
	{
		return;
	}

	struct InlineSymbolStack *new_last_inline_symbol_stack = last_inline_symbol_stack->prev;

	free(last_inline_symbol_stack);

	if (last_inline_symbol_stack == first_inline_symbol_stack)
	{
		first_inline_symbol_stack = last_inline_symbol_stack = NULL;
		return;
	}

	new_last_inline_symbol_stack->next = NULL;
	last_inline_symbol_stack = new_last_inline_symbol_stack;	
}

struct IncludePath
{
	char *path;

	struct IncludePath *next_element;
};

struct IncludePath *first_include_path, *last_include_path;

void add_include_path(char *path)
{
	struct IncludePath *new_element;

	new_element = (struct IncludePath*)malloc(sizeof(struct IncludePath));
	new_element->path = path;
	new_element->next_element = NULL;

	if (last_include_path == NULL)
	{
		first_include_path = last_include_path = new_element;
	}
	else
	{
		last_include_path->next_element = new_element;
		last_include_path = new_element;
	}
}

BOOL get_file_include_path(char *output_file_path, char *file_path, char* origin_file_path)
{
	FILE *fp = NULL;
	struct IncludePath *current_include_path = first_include_path;

	char temp_str_path[256];

	filename_get_path(temp_str_path, origin_file_path);
	filename_add_path(output_file_path, file_path, temp_str_path);

	do
	{
		fp = fopen(output_file_path, "r");
		if (fp)
		{
			fclose(fp);
			return TRUE;
		}

		if (current_include_path == NULL)
		{
			return FALSE;
		}

		filename_add_path(output_file_path, file_path, current_include_path->path);
		current_include_path = current_include_path->next_element;
	} while(TRUE);

	return FALSE;
}

struct DefineIdentifier
{
	char *identifier;
	int identifier_size;

	struct DefineIdentifier *next_element;
};

struct DefineIdentifier *first_define_identifier = NULL;

BOOL has_define_identifier(char *identifier, int identifier_size)
{
	struct DefineIdentifier *current_element = first_define_identifier;

	while(current_element != NULL)
	{
		if (is_str_equal2(current_element->identifier, current_element->identifier_size, identifier, identifier_size))
		{
			return TRUE;
		}
		current_element = current_element->next_element;
	}

	return FALSE;
}

void add_define_identifier(char *identifier, int identifier_size)
{
	struct DefineIdentifier *new_element;

	write_debug("Setting define identifier %.*s", identifier_size, identifier);

	new_element = (struct DefineIdentifier*)malloc(sizeof(struct DefineIdentifier));
	new_element->identifier = identifier;
	new_element->identifier_size = identifier_size;
	new_element->next_element = NULL;

	if (first_define_identifier != NULL)
	{
		new_element->next_element = first_define_identifier;
	}

	first_define_identifier = new_element;	
}

struct IfdefExpect
{
	enum IfdefExpectType type;

	struct IfdefExpect *next, *prev;
};

static struct IfdefExpect *first_ifdef_expect = NULL, *last_ifdef_expect = NULL;
static struct IfdefExpect *dup_first_ifdef_expect = NULL, *dup_last_ifdef_expect = NULL;

void push_ifdef_expect(int type)
{
	struct IfdefExpect *new_ifdef_expect = (struct IfdefExpect*)malloc(sizeof(struct IfdefExpect));
	new_ifdef_expect->type = type;
	new_ifdef_expect->next = NULL;
	new_ifdef_expect->prev = last_ifdef_expect;

	if (first_ifdef_expect == NULL)
	{
		first_ifdef_expect = new_ifdef_expect;
	}
	else
	{
		first_ifdef_expect->next = new_ifdef_expect;
	}

	last_ifdef_expect = new_ifdef_expect;
}

enum IfdefExpectType peek_ifdef_expect()
{
	if (last_ifdef_expect == NULL)
	{
		return IFDEF_EXPECT_UNKNOWN;
	}

	return last_ifdef_expect->type;
}

void pop_ifdef_expect()
{
	if (last_ifdef_expect == NULL)
	{
		return;
	}

	struct IfdefExpect *new_ifdef_expect = last_ifdef_expect->prev;

	free(last_ifdef_expect);

	if (last_ifdef_expect == first_ifdef_expect)
	{
		first_ifdef_expect = last_ifdef_expect = NULL;
		return;
	}

	new_ifdef_expect->next = NULL;
	last_ifdef_expect = new_ifdef_expect;	
}

void save_duplicate_all_ifdef_expect()
{
	struct IfdefExpect *current_ifdef_expect = first_ifdef_expect;
	struct IfdefExpect *dup_current_ifdef_expect = NULL;

	while(current_ifdef_expect != NULL)
	{
		struct IfdefExpect *new_ifdef_expect = (struct IfdefExpect*)malloc(sizeof(struct IfdefExpect));
		new_ifdef_expect->type = current_ifdef_expect->type;
		new_ifdef_expect->prev = dup_current_ifdef_expect;
		new_ifdef_expect->next = NULL;

		if (dup_current_ifdef_expect == NULL)
		{
			dup_first_ifdef_expect = new_ifdef_expect;
		}
		dup_current_ifdef_expect = new_ifdef_expect;

		current_ifdef_expect = current_ifdef_expect->next;
	}

	dup_last_ifdef_expect = dup_current_ifdef_expect;
}

void revert_to_duplicate_ifdef_expect()
{
	struct IfdefExpect *current_ifdef_expect = first_ifdef_expect;
	struct IfdefExpect *next_ifdef_expect;

	while(current_ifdef_expect != NULL)
	{
		next_ifdef_expect = current_ifdef_expect->next;
		free(current_ifdef_expect);
		current_ifdef_expect = next_ifdef_expect;
	}

	first_ifdef_expect = dup_first_ifdef_expect;
	last_ifdef_expect = dup_last_ifdef_expect;

	dup_first_ifdef_expect = NULL;
	dup_last_ifdef_expect = NULL;
}
