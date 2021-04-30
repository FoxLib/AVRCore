
            org     0000h               ; F000:0000

            mov     ax, $b800
            mov     ds, ax
            mov     ax, $3061
            mov     cx, 2000
@@:         mov     word [bx],ax
            inc     bx
            inc     bx
            loop    @b

            hlt
