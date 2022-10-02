# Z80HLA Manual

## Directives

**#origin *expression***

Set the current address.

Example:
```
#origin 0x2000
```

**#include  *string***

Includes the content of a file.  
The file path is relative to the file that's including it or relative to one of the include paths defined as arguments.  

Example:
```
#include "msx.z80hla"
```

**#include_binary *string***

Includes the content of a file as binary data.
The file path is relative to the file that's including it or relative to one of the include paths defined as arguments. 

Example:
```
#include_binary "font.bin"
```

**#print *string|expression[*,*...]***

Outputs expression values and strings at compile-time.  

Example:
```
#print "Size of PlayerType: ", sizeof(PlayerType)
```

**#define *identifier***  
**#ifdef *identifier***  
**#ifndef *identifier***  
**#else**  
**#endif**  

Define identifiers that can be used to deactivate parts of the code.
These directives are the only ones that can be used anywhere in the code, including in the same line as other syntactic elements.  

Example:
```
#define DEBUG

ld hl, size
ld d, (hl)
ld bc, #ifdef DEBUG 0x2002 #else 0x2003 #endif
ld hl, start 
do
{
	ld a, (hl)
	out (c), a
	inc bc
	inc hl

	#ifdef DEBUG
		push bc
		ld bc, 0x0001
		out (c), a
		pop bc
	#endif

	dec d
} while(nz)
```


**#cpu_type *string***

Sets the Z80 cpu type instruction set, possible values are:
- `z80`: Typical Z80 cpu instruction set (default)
- `msx` or `r800`: R800 CPU instruction set
- `gb`: Gameboy Z80 CPU instruction set

Example:
```
#cpu_type "gb"

swap
```

**#output_on**  
**#output_off**

Enables or disables generating the output bytes.
By default, output is on.

Example:
```
#origin 0

ld a, 0
ld (flag), a
forever
{    
}

#origin 0x8000
#output_off

data byte flag[1]
```

**#assembleall_on**  
**#assembleall_off**

Enables or disables assembling functions and data in libraries not being referenced.
By default, it is off.

Example:
```
; by default #assembleall_off

library Misc
{
  function foo()
  {
    ld a, foo_data
  }

  function foo2()
  {
    ld a, (another_foo_data)
  }

  data byte foo_data
  data byte another_foo_data = 1, 2, 3, 4, 5
}

foo()
foo3()

#assembleall_on

library Misc
{
  function foo3()
  {
    nop
  }

  function foo4()
  {
    ld hl, foo_data2
  }

  data word foo_data2[10]
}

; Misc::foo, Misc::foo2, Misc:foo3, Misc:foo4 and Misc::foo_data2 is assembled
; Misc::foo2 and Misc::another_foo_data are ignored
```

## Comments

**;** or **//**

Line comment.

Example:
```
ld a, 10
; This a comment
jp main
// This is also a comment
```
	
**/\* ... \*/**:

Multi-line comment (no nested multi-line comments supported).

Example:
```
/*
a multi-line
comment
*/
```

## Assembler

All Z80 opcodes are supported, including non-documented intructions and registers (for the CPU types that support them).

Example:
```
; cpu type is standard Z80 by default
ld ixl, 100
ex af, af'
ld a, 10    ; immediate value
ld a, (100) ; value in memory position 100

#cpu_type "msx"

ld a, (value)
ld b, 25
mulub a, b

#cpu_type "gbz80"

stop
swap b
```

The instruction `db` can be used to simply dump bytes, it does however only support a maximum of 16 operands.

Example:
```
db 100  ; outputs the byte 100
db 0, 1 ; outputs the bytes 0 and 1
```

## High-level syntax

### Code blocks

**function**

Defines a function without arguments.
Functionally the same as declaring a label and ending with `ret`.  

Example:
```
function do_stuff()
{
  ld a, (value)
  ld (data+2), a
}

do_stuff()
call do_stuff
```
is equivalent to  
```
do_stuff:
  ld a, (value)
  ld (data+2), a
  ret

  call do_stuff
  call do_stuff
```

**interrupt**

Defines an interrupt function.  
Functionally it's the same as declaring a label and ending the block with `reti` or `retn` (for interrupts named *nmi* and if available for the given cpu type).

Example:
```
#origin 0x66

interrupt nmi()
{
  ld a, (int_counter)
  inc a
  ld (int_counter), a
}
```
is equivalent to  
```
#origin 0x66

nmi:
  ld a, (int_counter)
  inc a
  ld (int_counter), a
  reti
```

***inline***

Defines a block of code that will be included wherever it's called.  
Arguments are not supported.  

Example:
```
jp main
inline print_a()
{
  ld bc, OUTPUT_CHAR
  out (c), a
}

main:
  print_a() 
```
is equivalent to  
```
  jp main
main:
  ld bc, OUTPUT_CHAR
  out (c), a
```

### Branching

**if (*cond*) {...} [else {...}]**  

Branching using conditional `jr` whenever possible and conditional `jp` in the other cases.  
The conditions that can be used are those possible with the Z80 cpu: `z`, `nz`, `c`, `nc`, `p`, `m`, `pe`, `po`.  

Example:

```
dec b
if (nz)
{
	inc a
}
else
{
	ld a, (initial_value)
}
```  
is equivalent to  
```
  dec b
  jr z, label
  inc a
  jr label2
label:
  inc a
  jr label2
  ld a, (initial_value)
label2:
```

### Loops

**while (*cond*) {...}**  
**do {...} while(*cond*)**  

Conditional loop using `jr` when possible and `jp`otherwise.  

Example:
```
ld d, 100
ld a, 0xFF
do
{
	out (c), a
	dec d
} while(nz)
in a, (c)
cp 0
while (nz)
{
  dec a
}
```

**forever {...}**

Infinite loop using `jr` when possible and `jp` otherwise.

Example:
```
forever()
{
  do_stuff()
}
```
is equivalent to  
```
loop:
  call do_stuff
  jr loop
```

***break***

Jumps out of the current loop.
It always uses a `jp` to do so (it could be more efficient to use a `jr` in some occasions).

```
forever()
{
  ld a, (value)
  if (z)
  {
    break
  }
}
```

***breakif(cond)***

Jumps out of the current loop under a condition.
It always uses a `jp` to do so.

```
forever()
{
  ld a, (value)
  breakif(z)
}
```

### Data declaration

Data declaration outputs to the resulting file the bytes that it declares (unless #output_off is activated, see Directives) and it advances the current address.
The possible types are:
- `byte`: 1 byte
- `word`: 2 bytes
- `dword`: 4 bytes
- *custom structured type*: a custom structured type that has been declared

The name of the data declarations can be used as a symbol that contains its address and can also be used in `sizeof` and `length` expression elements (see Expressions).

**data *type* *[name]* = *expression*|*string*,...**  

Example of declaring an anonymous byte:
```
data byte = 10
```
this is equivalent to
```
db 10
```

Example of declare a byte array using a string:
```
data byte text = "Hello you", 0
```
this is equivalent to
```
text:
  db 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x79, 0x6f, 0x75, 0x00
```

Example of declaring an array of words:
```
data word stuff = 0, 1000, 10000, 100, 2467
```
this is equivalent to
```
stuff:
  db 0, 0
  db 0xe8, 0x03
  db 0x10, 0x27
  db 0x64, 0x00
  db 0xA3, 0x09
```

**data *type* *[name]* [*expression*]**  

Declare uninitialized data (filled with 0).

Example:
```
data byte [20]        ; 20 bytes
data word score       ; 2 bytes named "score"
data byte table[100]  ; 100 bytes name "table"
```


**data *type* *[name]* from *string***

Declare data using binary file contents.  
The file size must be a multiple of the size of the declared type.  
The file path is relative to the file that's including it or relative to one of the include paths defined as arguments.  

Example:
```
data byte sin_table from "sin_table.bin"
```

### Structure types

It is possible to declare structure types (`struct` and `union`) to be used in data declaration.

**struct|union *identifier* {**  
***type* *identifier*, ...**  
**...**  
**}**

`struct` types are similar to C-type structs, each field occupies the space in memory contiguous to the last field.
```
struct Player
{
	byte x, y           ; occupies relative position 0 and 1 of memory
	byte lives          ; occupies relative position 2 of memory
	word score          ; occupies relative position 3 and 4 of memory
	byte attributes[5]  ; occupies relative position 5, 6, 7, 8 and 9 of memory
}

ld a, sizeof(Player)  ; = 9
```

`union` types are similar to C-type unions, each field start in the same relative memory position.
```
union Value
{
	byte byte_value		; occupies relative position 0 of memory
	word word_value		; occupies relative position 0 and 1 of memory
	dword dword_value	; occupies relative position 0, 1, 2 and 3 of memory
}

ld a, sizeof(Value)  ; = 4
```

#### Declaring variables of structure types

```
struct Vehicle
{
  byte type
  word max_speed
  union {
    struct {
      byte tire_condition[4]
    } car
    struct {
      byte wing_span
      word landing_distance_required
    } airplane
    struct {
      word max_cargo
    } ship
  } attributes
}

const VEHICLE_UNKNOWN = 0
const VEHICLE_CAR = 1
const VEHICLE_AIRPLANE = 2
const VEHICLE_SHIP = 3

data Vehicle vehicles = {
  type = VEHICLE_CAR
  max_speed = 100
  attributes = {
    car = {
      tire_condition = 100, 98, 90, 100
    }
  }
}, {
  type = VEHICLE_AIRPLANE
  max_speed = 1000
  attributes = {
    airplane = {
      wing_span = 200
      landing_distance_required = 10
    }
  }
}, {
  type = VEHICLE_SHIP
  max_speed = 200
  attributes = {
    ship = {
      max_cargo = 1500
    }
  }
}, {
  type = VEHICLE_UNKNOWN
}, {}, {}

data Vehicle more_vehicles[10]

#print sizeof(Vehicle)                                      ; 7
#print sizeof(vehicles), " ", length(vehicles)              ; 42 6
#print sizeof(more_vehicles), " ", length(more_vehicles)    ; 70 10
```

#### Referencing field

Getting the field address of data.

**data.*field*|*field[index]*...**

Getting the field relative address of a type

**type.*field*|*field[index]*...**

Example
```
; Using the declaration in the example above

ld hl, 2000
; absolute address of field of data
ld (more_vehicles[5].attributes.max_cargo), hl 

; relative address of a field of a type
#print Vehicle.type   ; 0
```



### Libraries

Libraries contain functions, inlines, data declaration and struct types.
Their full names are *library name*::*name*.  
Inside the library where they're declared they can be referenced by their short or full name. Outside the library the full name needs to be used.

**library *name* {**  
***...function|data|const...***  
**}**

Example:
```
library MyLibrary
{
  const my_const = 100

  function my_func()
  {
    ld a, 100
    ld bc, 0x1000
    out (c), a
  }

  data byte my_value = 100
}
```

Libraries can be declared in parts

```
#origin 0x0000
library MyLibrary
{
  function func()
  {
    ld a, 100
    ld (value), a
  }
}

MyLibrary::func()

library MyLibrary
{
  data byte value = 101
}
```

One of the main features of libraries are that functions and data not used are not assembled unless the directive `#assembleall_on` is used or the corresponding option is used.  
Only those used directly or indrectly are assembled, the rest are ignored.

```
data byte value

library Misc
{
  data byte value1
  data byte value2

  function func1()
  {
    func3()
  }

  function func2()
  {
    ld a, 100
    ld (value1), a
  }

  function func3()
  {
    ld a, (value2)
    ld (value), a
  }
}

Misc::func1()

; Misc::func1, Misc::func3 and Misc::value2 will be assembled
; Misc::value1 and Misc::func2 will be ignored
```


### Expressions

**Numbers**

```
ld a, 100   ; Decimal
ld a, 0x100 ; Hexadecimal
ld a, 0b111 ; Binary
ld a, 0o777 ; Octal
```

**Characters**

```
ld a, 'C'
out (c), a
ld a, '\n'
out (c), a
```

**Operators**

Arithmetic: `+`, `-`, `*`, `/`, `%`  

Bitwise: `~`, `|`, `&`, `^`, `<<`, `>>`  

**Keywords**

`$` returns the current address

```
data byte padding[0x100-$]
```

`sizeof` returns the size in bytes of a data declaration or a type.  

```
data word position[2]

ld a, sizeof(position) ; = 4

struct Player
{
	byte pos_x, pos_y
	byte life
	byte type
	union {
		word wizard
		word warrior
		word rogue
	} stats
}

ld a, sizeof(Player) ; = 6
```

`length` returns the number of elements of a data declaration.  

```
data byte position = 100, 100

ld a, length(position) ; = 2

data StructType entities[10]

ld a, length(entities) ; = 10
```

**Symbols**

Constants can be referenced and their value is returned

```
const value = 10

ld a, value ; = 10
```

Labels and function names return their address

```
#origin 0x0000

start:

	ld a, 10
	ld bc, 0x1000
	out (c), a

 	function foo()
	{
		ld a, 1
	}

	ld hl, start      ; = 0
	ld a, foo * 2     ; = 7 * 2 = 14
```

Symbols inside libraries can be referenced by their full names

```
library Misc
{
  data byte value = 10

  function func()
  {
    ld (value), a
    ld bc, 0x1000
    out (c), a
  }
}

ld a, 100
ld (Misc::value), a
ld hl, Misc::func
```

## Suggestions of use

The assembler will only take one input file.  
If the program is too large and comprised of multiple files, it is advisable to create one file as the entry point and include the other files.

Example of main file for a system with ROM from 0x0000 to 0x7FFF and RAM from 0x8000 onwards.  
The code and readonly data sits in ROM and is assembled, the data sits in RAM and is not assembled (hence the use of `#output_off`)

```
// Start of ROM
#origin 0x0000

data byte [0x066-$]
#include "nmi.z80hla"

#include "library_console_code.z80hla"
#include "game_utils_code.z80hla"
#include "game_logic_code.z80hla"

#print "End of code: ", $

#include "game_tables.z80hla"

#print "End of readonly data: ", $

// Start of RAM
#origin 0x8000
#output_off

#include "library_console_data.z80hla"
#include "game_data.z80hla"

#print "End of RAM data: ", $
```

There's also the possibility of generating several binary files from a simple compilation.  
For this one only needs to use the directive `#output_file` several times.  
Example:
```
#origin 0x0000
#output_file "rom1.bin"
#include "rom1.z80hla"

#origin 0x0000
#output_file "rom2.bin"
#include "rom2.z80hla"

#origin 0x8000
#output_file "rom3.bin"
#include "rom3.z80hla"
```
