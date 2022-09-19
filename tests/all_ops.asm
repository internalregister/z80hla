org 0

adc a, b
adc a, (hl)
adc a, (ix+1)
adc a, (iy+100)
adc a, (iy-30)
adc a, (ix+1)
adc a, (ix-1)
adc a, 255
adc a, 0
adc a, 135
adc a, b
; adc a, ixh
; adc a, iyl
adc hl, bc
adc hl, de
adc hl, bc
adc hl, hl
adc hl, sp
adc a, h
add a, (hl)
add a, (ix+4)
add a, (ix-4)
add a, (iy+40)
add a, (iy-40)
add a, 100
add a, 255
add a, a
add a, h
add a, c
add hl, bc
add hl, de
add hl, hl
add hl, sp
add ix, bc
add ix, de
add ix, ix
add ix, sp
add iy, bc
add iy, de
add iy, iy
add iy, sp
and (hl)
and (ix+5)
and (ix-5)
and (iy+105)
and (iy-105)
and 250
and a
and b
and l
and c
; and ixl
; and ixh
; and iyl
; and iyh
bit 2, (hl)
bit 6, (ix + 5)
bit 6, (ix - 15)
bit 3, a
bit 3, h
bit 3, l
call 1000
call c, 3000
call nc, 3000
call z, 3000
call nz, 3000
call m, 3000
call p, 3000
call po, 3000
call pe, 3000
ccf
cp (hl)
cp (ix+50)
cp (ix-50)
cp (iy+50)
cp (iy-50)
cp 55
cp h
cp b
; cp ixh
; cp ixl
; cp iyh
; cp iyl
cpd
cpdr
cpi
cpir
cpl
daa
dec (hl)
dec (ix + 5)
dec (ix - 100)
dec (iy + 5)
dec (iy - 100)
dec a
dec b
dec bc
dec c
dec d
dec de
dec e
dec h
dec hl
dec ix
dec iy
; dec ixl
; dec ixh
; dec iyl
; dec iyh
dec l
dec sp
di
djnz $+30
ei
ex (sp), hl
ex (sp), ix
ex (sp), iy
ex af, af'
ex de, hl
exx
halt
im 0
im 1
im 2
in a, (c)
in a, (100+100)
in b, (c)
in c, (c)
in d, (c)
in e, (c)
in h, (c)
in l, (c)
in f, (c)
inc a
inc b
inc bc
inc c
inc d
inc de
inc e
inc h
inc hl
inc ix
inc iy
; inc ixl
; inc ixh
; inc iyl
; inc iyh
inc l
inc sp
ind
indr
ini
inir
jr $+10
jr c, $+20
jr nc, $-5
jr z, $+10
jr nz, $+20
ld (bc), a
ld (de), a
ld (hl), 100
ld (hl), b
ld (ix+100), 100
ld (ix-100), 101
ld (ix+100), b
ld (iy+100), 100
ld (iy-100), 101
ld (iy+100), b
ld (10001), a
ld (100), bc
ld (100), de
ld (100), hl
ld (100), ix
ld (100), iy
ld (100), sp
ld a, (bc)
ld a, (de)
ld a, (hl)
ld a, (ix + 100)
ld a, (ix - 100)
ld a, (iy + 100)
ld a, (iy - 100)
ld a, (10001)
ld a, 100
ld a, b
; ld a, ixl
; ld a, ixh
; ld a, iyl
; ld a, iyh
ld a, i
ld a, r
ld b, (hl)
ld b, (ix+100)
ld b, (ix-100)
ld b, (iy+100)
ld b, (iy-100)
ld b, 100
ld b, b
; ld b, ixl
; ld b, ixh
; ld b, iyl
; ld b, iyh
ld bc, (10001)
ld bc, 10001
ld c, (hl)
ld c, (ix+100)
ld c, (ix-100)
ld c, (iy+100)
ld c, (iy-100)
ld c, 100
ld c, c
; ld c, ixl
; ld c, ixh
; ld c, iyl
; ld c, iyh
ld d, (hl)
ld d, (ix+100)
ld d, (ix-100)
ld d, (iy+100)
ld d, (iy-100)
ld d, 100
ld d, d
; ld d, ixl
; ld d, ixh
; ld d, iyl
; ld d, iyh
ld de, (10001)
ld de, 10001
ld e, (hl)
ld e, (ix+100)
ld e, (ix-100)
ld e, (iy+100)
ld e, (iy-100)
ld e, 100
ld e, d
; ld e, ixl
; ld e, ixh
; ld e, iyl
; ld e, iyh
ld h, (hl)
ld h, (ix+100)
ld h, (ix-100)
ld h, (iy+100)
ld h, (iy-100)
ld h, 100
ld h, d
ld hl, (10001)
ld hl, 10001
ld i, a
ld ix, (10001)
ld ix, 10001
; ld ixh, 100
; ld ixh, ixl
; ld ixl, 100
; ld ixl, ixh
ld iy, (10001)
ld iy, 10001
; ld iyh, 100
; ld iyh, iyl
; ld iyl, 100
; ld iyl, iyh
ld l, (hl)
ld l, (ix+100)
ld l, (ix-100)
ld l, (iy+100)
ld l, (iy-100)
ld l, 100
ld l, d
ld r, a
ld sp, (10001)
ld sp, hl
ld sp, ix
ld sp, iy
ld sp, 10001
ldd
lddr
ldi
ldir
neg
nop
or (hl)
or (ix+5)
or (ix-5)
or (iy+105)
or (iy-105)
or 250
or a
or b
or l
or c
; or ixl
; or ixh
; or iyl
; or iyh
otdr
otir
out (c), a
out (c), b
out (c), c
out (c), d
out (c), e
out (c), h
out (c), l
out (100), a
outd
outi
pop af
pop bc
pop de
pop hl
pop ix
pop iy
push af
push bc
push de
push hl
push ix
push iy
res 3, (hl)
res 5, (ix+100)
res 5, (ix-100)
res 4, (iy+100)
res 4, (iy-100)
res 6, b
rl (hl)
rl (ix+100)
rl (ix-100)
rl (iy+100)
rl (iy-100)
rl b
rla
rlc (hl)
rlc (ix+100)
rlc (ix-100)
rlc (iy+100)
rlc (iy-100)
rlc b
rlca
rld
rr (hl)
rr (ix+100)
rr (ix-100)
rr (iy+100)
rr (iy-100)
rr b
rra
rrc (hl)
rrc (ix+100)
rrc (ix-100)
rrc (iy+100)
rrc (iy-100)
rrc b
rrca
rrd
rst 0
rst 8
rst 16 ; 0x10
rst 24 ; 0x18
rst 32 ; 0x20
rst 40 ; 0x28
rst 48 ; 0x30
rst 56 ; 0x38
sbc a, b
sbc a, (hl)
sbc a, (ix+1)
sbc a, (iy+100)
sbc a, (iy-30)
sbc a, (ix+1)
sbc a, (ix-1)
sbc a, 255
sbc a, 0
sbc a, 135
sbc a, b
; sbc a, ixh
; sbc a, iyl
sbc hl, bc
sbc hl, de
sbc hl, bc
sbc hl, hl
sbc hl, sp
sbc a, h
scf
set 3, (hl)
set 5, (ix+100)
set 5, (ix-100)
set 4, (iy+100)
set 4, (iy-100)
set 6, b
sla (hl)
sla (ix+100)
sla (ix-100)
sla (iy+100)
sla (iy-100)
sla b
sra (hl)
sra (ix+100)
sra (ix-100)
sra (iy+100)
sra (iy-100)
sra b
srl (hl)
srl (ix+100)
srl (ix-100)
srl (iy+100)
srl (iy-100)
srl b
sub (hl)
sub (ix+5)
sub (ix-5)
sub (iy+105)
sub (iy-105)
sub 250
sub a
sub b
sub l
sub c
; sub ixl
; sub ixh
; sub iyl
; sub iyh
xor (hl)
xor (ix+5)
xor (ix-5)
xor (iy+105)
xor (iy-105)
xor 250
xor a
xor b
xor l
xor c
; xor ixl
; xor ixh
; xor iyl
; xor iyh
