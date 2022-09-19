# Z80HLA - Z80 High-level assembler

A high-level assembler for Z80 CPUs.

## Introduction

An assembler targeting Z80 CPUs with some high-level syntax that is translated into simple and direct assembly instructions.  

The idea is for the binary generated to be equally efficient as if written in pure assembly, but also having a few advantages (in my opinion), for example:
* increase readability by not having as many labels to cope with loops and branching
* having data well defined and with types (including structs and unions)
* having the ability to create libraries of code that are located in a specific *namespace* and is only generated if used (see `library` in the language [manual](MANUAL.md)).  

It supports the instruction sets of the standard Z80 CPU with its undocumented instructions, the R800 CPU (used in the MSX turboR) and the Sharp LR35902 (used in the Nintendo Gameboy and Nintendo Gameboy Color consoles).  

This assembler was designed simply as a hobby project, inspired in part by the [NESHLA](http://neshla.sourceforge.net/) project by Brian Provinciano.  

It's written in C having only the standard library as a dependency.  

It was originally initially only intended to be used in my own projects for my own Z80 systems, however I made it compatible with a wider spectrum of Z80 CPUs and decided to make open-source and available to anyone who for some reason would wish to use such a tool.  

## Building it

In UNIX environments use `make`.  

In Windows if you're not using a UNIX environment such as WSL you can use Visual Studio and open the `z80hla.sln` solution file.  

The build will result in a `z80hla` or `z80hla.exe` binary file in the `bin` directory.  

You can execute it from here or copy it to a location where you can use it from any directory.  

## Using it

To use it simply do:
```
z80hla file.z80hla
```

If there are no errors, an `output.bin` will be the result.

Here are some extra options you can use when executing the assembler:

- `-o`/`--output` `FILE` : set the output file
- `-i`/`--include` `PATH` : add an include path
- `-c`/`--cpu` `CPU` : set a CPU type: `z80` (default), `gb`, `msx` or `r800` (the same as `msx`)
- `-d`/`--define` `IDENTIFIER` : add a define identifier
- `-s`/`--symbols` `FILE` : output a JSON file with all the symbols
- `-l`/`--list` `FILE` : output a file listing instructions and data without high-level constructs

You can find the manual for the Z80HLA language [here](MANUAL.md).

There is also code for syntax highlighting plugins for text editors in the folder `editors`.

## Extra Resources

In the directory `examples` you can find example programs for the Nintendo Gameboy (`gb`) and Sega Master System (`sms`) consoles as well as for my own [homebrew videogame console]() (https://internalregister.github.io/2019/03/14/Homebrew-Console.html) (`homebrew_console`).

## License

Z80HLA is licensed under a 2-clause BSD license
