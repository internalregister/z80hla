org 0

jp main

FunctionA:
    call FunctionB
    ret

FunctionB:
    call FunctionA
    call FunctionA
    ret

call FunctionA
ld a, 100

main:
    call FunctionB
    call FunctionA
    jr main
