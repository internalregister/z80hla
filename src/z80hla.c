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

void write_error(char *fmt, ...)
{
	va_list arg_ptr;

	printf("[ERROR] ");
	va_start(arg_ptr, fmt);
	vprintf(fmt, arg_ptr);
	va_end(arg_ptr);
	printf("\n");
}

void write_compiler_error(char *filename, int current_line, char *fmt, ...)
{
	va_list arg_ptr;

	printf("[ERROR] %s:%d: ", filename, current_line);
	va_start(arg_ptr, fmt);
	vprintf(fmt, arg_ptr);
	va_end(arg_ptr);
	printf("\n");
}

void write_debug_impl(char *fmt, ...)
{
	va_list arg_ptr;

	printf("[DEBUG] ");
	va_start(arg_ptr, fmt);
	vprintf(fmt, arg_ptr);
	va_end(arg_ptr);
	printf("\n");
}

void print_usage()
{
	printf("Usage: z80hla [options] input_file\n\n");
	printf("Options:\n");
	printf("-o\t--output FILE\t\tSet the default output file.\n");
	printf("-i\t--include PATH\t\tAdd include path.\n");
	printf("-c\t--cpu CPU_TYPE\t\tSet CPU type: \"z80\" (default), \"gb\", \"msx\" / \"r800\"\n");
	printf("-d\t--define IDENTIFIER\tAdd define identifier\n");
	printf("-s\t--symbols FILE\t\tOutput symbols JSON file\n");
	printf("-l\t--list FILE\t\tOuput listing of instructions without high-level constructs\n");
	printf("\n");
}

enum CPUType cpu_type = CPU_TYPE_Z80, initial_cpu_type = CPU_TYPE_Z80;
FILE *fp_list = NULL;

int main(int argc, char *argv[])
{
	char *input_filename = NULL;	
	char *symbol_filename = NULL;
	char *listing_filename = NULL;	

	printf("Z80 high-level assembler - Sérgio Vieira 2022\n\n");

	if (argc == 1)
	{
		print_usage();
		return 0;
	}

	compiler_output_filename = "output.bin";

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}
			compiler_output_filename = argv[i];
		}
		else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--include"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}
			add_include_path(argv[i]);
		}
		else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--cpu"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}

			if (!strcmp(argv[i], "z80"))
			{
				initial_cpu_type = cpu_type = CPU_TYPE_Z80;
			}
			else if (!strcmp(argv[i], "gb"))
			{
				initial_cpu_type = cpu_type = CPU_TYPE_GB;
			}
			else if (!strcmp(argv[i], "msx") || !strcmp(argv[i], "r800"))
			{
				initial_cpu_type = cpu_type = CPU_TYPE_MSX;
			}
			else
			{
				printf("Error: Invalid CPU type \"%s\"\n", argv[i]);
				return 1;
			}
		}
		else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--define"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}

			add_define_identifier(argv[i], strlen(argv[i]));			
		}
		else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--symbols"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}

			symbol_filename = argv[i];
		}
		else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list"))
		{
			i++;
			if (i == argc)
			{
				printf("Error: Option requires an argument \"%s\"\n", argv[i-1]);
				return 1;
			}

			listing_filename = argv[i];
			fp_list = fopen(listing_filename, "wt");

			if (!fp_list)
			{
				printf("Cannot open file \"%s\" to write\n", listing_filename);
				return 1;
			}
		}
		else
		{
			input_filename = argv[i];
		}
	}

	if (input_filename == NULL)
	{
		printf("Error: No input file\n");
		return 1;
	}

	struct Lexer lexer;
	if (init_lexer(&lexer, input_filename))
	{
		return 1;
	}
	
	push_include_file(lexer.filename);
	struct ASTNode *node = parse(&lexer, NULL, NULL);
	pop_include_file();

	if (node != NULL)
	{		
		fill_library_symbols_used_with_dependencies();

		#if DEBUG == 1
		FILE *fp_AST = fopen("output_ast.json", "w");
		fprint_ast(fp_AST, node);
		fclose(fp_AST);

		FILE *fp_dependencies = fopen("output_dependencies.json", "w");
		fprint_library_symbol_dependencies(fp_dependencies);
		fclose(fp_dependencies);

		FILE *fp_used_library_symbols = fopen("output_library_symbols_used.json", "w");
		fprint_library_symbols_used(fp_used_library_symbols);
		fclose(fp_used_library_symbols);
		#endif

		init_compiler();
		if (compile(node))
		{
			write_error("Error compiling");

			if (fp_list)
			{
				fclose(fp_list);
				if (remove(listing_filename))
				{
					write_error("Unable to remove file \"%s\"", listing_filename);
				}
				return 1;
			}
		}
		else
		{
			#if DEBUG == 1			
			FILE *fp_out = fopen("output_output.json", "w");
			fprint_output(fp_out);
			fclose(fp_out);
			#endif

			if (fp_list)
			{
				fclose(fp_list);
			}
		}

		#if DEBUG == 1
		FILE *fp_constants = fopen("output_constants.json", "w");
		fprint_constants(fp_constants);
		fclose(fp_constants);

		FILE *fp_structs = fopen("output_structs.json", "w");
		fprint_structured_types(fp_structs);
		fclose(fp_structs);

		FILE *fp_data = fopen("output_data.json", "w");
		fprintf_data_symbols(fp_data);
		fclose(fp_data);
		#endif

		if (symbol_filename != NULL)
		{
			FILE *fp_symbols = fopen(symbol_filename, "w");
			if (!fp_symbols)
			{
				printf("Cannot open file \"%s\" to write\n", symbol_filename);
				return 1;
			}
			fprintf_output_symbols(fp_symbols);
			fclose(fp_symbols);
		}
	}
	else
	{
		write_error("Error parsing file");
	}

	destroy_lexer(&lexer);

	printf("Compilation successful\n");

	return 0;
}

