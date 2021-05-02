
            org     0000h               ; F000:0000

            mov     ax, $b800
            mov     ds, ax
            mov     ax, $1700
            mov     cx, 2000
            call    t1
@@:         mov     word [bx],ax
            add     al, 1
            inc     bx
            inc     bx
            dec     cx
            jnz     @b
            hlt

t1:         mov     [3998], word $2050
            ret
