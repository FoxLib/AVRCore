
            org     0000h               ; F000:0000

            mov     ax, $b800
            mov     es, ax
            mov     ax, $1701
m1:         xor     di, di
            mov     cx, 2000
@@:         stosw
            add     al, 1
            xor     al, bl
            rol     bx, 1
            xor     bh, bl
            inc     bx
            loop    @b
            jmp     m1
            hlt
