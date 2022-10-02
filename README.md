# Z80HLA - Z80 High-level assembler

A high-level assembler for Z80 CPUs.

## Introduction

An assembler targeting Z80 CPUs with some high-level syntax that is translated into simple and direct assembly instructions.  

Example of the syntax:
```
#define DEBUG
#origin 0x0000

jp main

data byte [0x66-$]
interrupt nmi()
{
}

library MyLibrary
{
    struct MyStruct
    {
        byte x, y
        union {
            word full
            struct {
                byte low, high
            } partial
        } value
    }

    const OUT_CHAR = 0x1000

    function func()
    {
        ld a, sizeof(my_array)
        ld hl, my_array
        do {
            ld (hl), 1
            inc hl
            dec a
        } while(nz)
#ifdef DEBUG
        ld a, length(my_array)
        ld bc, OUT_CHAR
        out (c), a
#endif
    }
}

#include "other_code.z80hla"

function func()
{
    MyLibrary::func()
}

main:
  func()
  ld hl, my_struct_var.value.partial.low
  ld (hl), 100
  forever
  {
  }

data byte sin_table from "sin_table.bin"

#origin 0x8000
#output_off

data MyLibrary::MyStruct my_struct_var = {}
library MyLibrary
{
    data byte my_array[100]
}
```

[More extensive program written in Z80HLA](https://github.com/internalregister/z80hla/blob/master/examples/homebrew_console/minibreakout/minibreakout.z80hla)

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

- `-h`/`--help` : Show usage information
- `-o`/`--output` `FILE` : set the output file
- `-i`/`--include` `PATH` : add an include path
- `-c`/`--cpu` `CPU` : set a CPU type: `z80` (default), `gb`, `msx` or `r800` (the same as `msx`)
- `-d`/`--define` `IDENTIFIER` : add a define identifier
- `-a`/`--assembleall` : Assemble all library symbols and not just the ones referenced
- `-s`/`--symbols` `FILE` : output a JSON file with all the symbols
- `-l`/`--list` `FILE` : output a file listing instructions and data without high-level constructs

You can find the manual for the Z80HLA language [here](MANUAL.md).

There is also code for syntax highlighting plugins for text editors in the folder [editors](https://github.com/internalregister/z80hla/tree/master/editors).

## Extra Resources

In the directory [examples](https://github.com/internalregister/z80hla/tree/master/examples) you can find example programs for several systems:
* [Nintendo Gameboy](https://github.com/internalregister/z80hla/tree/master/examples/gb)
* [Sega Master System](https://github.com/internalregister/z80hla/tree/master/examples/sms)
* [ZX Spectrum](https://github.com/internalregister/z80hla/tree/master/examples/zx_spectrum)
* [MSX](https://github.com/internalregister/z80hla/tree/master/examples/msx)
* [my own homebrew videogame console](https://github.com/internalregister/z80hla/tree/master/examples/homebrew_console) ([Link](https://internalregister.github.io/2019/03/14/Homebrew-Console.html) to blog about my first homebrew videogame console).

## License

Z80HLA is licensed under a 2-clause BSD license
