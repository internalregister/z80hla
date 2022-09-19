jp loop
single_byte:
db 100

rand:
    ld a, random_seed
label:
    nop
    ret

rand2:
    ld a, random_seed2
    call rand
    nop
    ret

loop:
    ld a, 120
    ld (single_byte), a
    call rand2
    jp label
    jp loop

random_seed:
dw 10000
random_seed2:
dw 10 * 10
