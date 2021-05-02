
            org     0000h               ; F000:0000

            mov     ax, $b800
            mov     es, ax
            mov     ax, $1701
@@:         xor     di, di
            mov     cx, 2000
            rep     stosw
            inc     ax
            jmp     @b
            hlt
