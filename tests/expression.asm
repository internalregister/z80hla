org 0

ld a, 100
ld a, 100
ld a, 100
ld a, 100

jp 4 / 2 * 4 + 3 - 1 + 2
jp 4 / 2 * (4 + 3) - 1 + 2
jp 4 / 2 * (4 + 3 - 1 + 2)
jp 4 / (2 * 4 + 3 - 1 + 2)

sub 1 + 1 * 2 + 2
sub 1 + (1 * 2) + 2
sub (1 + 1) * 2 + 2
sub (1 + 1) * (2 + 2)

ld hl, 2 | 1
ld hl, 2 & 1
ld hl, 2 & 1 | 3
ld hl, 2 & (1 | 3)
ld hl, (1 ^ 2)
ld hl, (1 << 10)
ld hl, 1 << 10 >> 10
ld hl, 1000 >> 5
ld hl, 1 << 1 + 5

add a, 65
add a, 39
jp z, '\0' + 9 + 122
