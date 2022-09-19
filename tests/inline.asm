org 0

label:
    ld a, 10
    ld b, 20
    sub b

label2:
    nop
    ld a, 10
    ld b, 20
    sub b
    nop
    ld a, 10
    ld b, 20
    sub b
    nop
    jp label
    jp label2
