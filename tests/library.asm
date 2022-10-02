jp loop
single_byte:
db 100

rand:
    ld hl, (random_seed)
label:
    nop
    ret

rand2:
    ld hl, (random_seed2)
    ld a, (array)
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
array:
db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

not_used:
    ld a, 55
    ld a, notused_data
    ret

notused_data:
    db 10, 100
