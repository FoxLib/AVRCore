
            org     0000h               ; F000:0000

            mov     ax, $b800
            mov     ds, ax
            mov     ax, $1700
            mov     [bx], ax

            hlt
