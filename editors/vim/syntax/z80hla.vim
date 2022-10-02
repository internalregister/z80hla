" Vim syntax file
" Language: Z80HLA
"

if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syntax keyword z80hlaConditional  if else
syntax keyword z80hlaRepeat       while do forever break breakif
syntax keyword z80hlaOtherKwords  break breakif 
syntax keyword z80hlaDeclaration  library function interrupt data struct
syntax keyword z80hlaInstruction  nop adc add and bit call ccf cp cpd cpdr cpi cpir cpl daa dec di djnz ei ex exx halt im
syntax keyword z80hlaInstruction  in inc ind indr ini inir jp jr ld ldd lddr ldi ldir neg or otdr otir out outd outi
syntax keyword z80hlaInstruction  pop push res ret reti retn rl rla rlc rlca rld rr rra rrc rrca rrd rst sbc scf set
syntax keyword z80hlaInstruction  sla sra srl sub xor
syntax keyword z80hlaInstruction  db sll swap stop ldh ldhl mulub muluw
syntax keyword z80hlaRegister     a b c d e f h l af af' bc de hl i r ix iy ixl ixh iyl iyh pc sp
syntax keyword z80hlaCond         nc m p z nz pe po
syntax keyword z80hlaType         byte word dword

syntax region	z80hlaBuiltin	start="\s*\zs\(%:\|#\)\s*\(include\>\|print\>\|define\>\|ifdef\>\|ifndef\>\|endif\>\|output_on\>\|output_off\>\|include_binary\>\|else\>\|cpu_type\>\|output_file\>\|assembleall_on\>\|assembleall_off\>\)" end="\s"

syntax match z80hlaLabel          "[a-zA-Z_][a-zA-Z_0-9]*:"
syntax match z80hlaNumber         "\v<\d%(_?\d)*"
syntax match z80hlaBinNumber      "\v<0b[01]%(_?[01])*"
syntax match z80hlaOctNumber      "\v<0o\o%(_?\o)*"
syntax match z80hlaHexNumber      "\v<0x\x%(_?\x)*"

syntax region z80hlaBlock start="{" end="}" transparent fold
syntax region z80hlaCommentLine start="//" end="$"
syntax region z80hlaCommentLine2 start=";" end="$"
syntax region z80hlaCommentBlock start="/\*" end="\*/"

syntax region z80hlaString matchgroup=z80hlaStringDelimiter start=+"+ skip=+\\\\\|\\"+ end=+"+ oneline contains=z80hlaEscape
syntax region z80hlaChar matchgroup=z80hlaCharDelimiter start=+'+ skip=+\\\\\|\\'+ end=+'+ oneline contains=z80hlaEscape
syntax match z80hlaEscape        display contained /\\./

highlight default link z80hlaInstruction Keyword
highlight default link z80hlaCommentLine Comment
highlight default link z80hlaCommentLine2 Comment
highlight default link z80hlaCommentBlock Comment
highlight default link z80hlaString String
highlight default link z80hlaStringDelimiter String
highlight default link z80hlaChar String
highlight default link z80hlaCharDelimiter String
highlight default link z80hlaEscape Special
highlight default link z80hlaType Type
highlight default link z80hlaOtherKwords Keyword
highlight default link z80hlaDeclaration Keyword
highlight default link z80hlaRepeat Repeat
highlight default link z80hlaConditional Conditional
highlight default link z80hlaBuiltin PreProc
highlight default link z80hlaRegister Identifier
highlight default link z80hlaCond Identifier
highlight default link z80hlaNumber Number
highlight default link z80hlaBinNumber Number
highlight default link z80hlaOctNumber Number
highlight default link z80hlaHexNumber Number
highlight default link z80hlaLabel Label

let b:current_syntax = "z80hla"

let &cpo = s:cpo_save
unlet! s:cpo_save

