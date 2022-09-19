org 0

nop10: macro
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
endm

nop100: macro
    nop10
    nop10
    nop10
    nop10
    nop10
    nop10
    nop10
    nop10
    nop10
    nop10
endm

    jr z, label0
    nop
    nop
label0:

    ld a, c
    cp 10

    jr nc, label1
    nop
    jr label2
label1:
    nop
    nop
    nop
label2:

    jp c, label3
    nop100
    nop100
    nop100
label3:

    jr nz, label4
    nop10
    jp label5
label4:
    nop100
    nop100
    nop100
    nop100
    nop100
label5:
