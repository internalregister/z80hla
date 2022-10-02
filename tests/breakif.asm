ld bc, 4096
ld a, 100

loop1:
    dec a
    jp z, loop1end 

    out (c), a
    jr loop1
loop1end:

nop
nop

loop2:
    jr nc, loop2end
    ld a, 100
loop3:    
    ld a, 77
loop4:    
    jr z, loop4end
    ld a, 200
    jp nc, loop4end
    ld b, a
    jr loop4
loop4end:    
    ld c, a
    jp p, loop3
loop3end:
    ld h, a
    jr loop2
loop2end:
