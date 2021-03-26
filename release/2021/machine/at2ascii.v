// https://ru.wikipedia.org/wiki/Скан-код
module at2ascii
(
    input  wire [7:0] at,
    input  wire       shift,
    output reg  [7:0] ascii
);

always @(*) begin

    case (at)

        // Цифробуквенная клавиатура
        /* A   */ 8'h1C: ascii = shift ? 8'h41 : 8'h61;
        /* B   */ 8'h32: ascii = shift ? 8'h42 : 8'h62;
        /* C   */ 8'h21: ascii = shift ? 8'h43 : 8'h63;
        /* D   */ 8'h23: ascii = shift ? 8'h44 : 8'h64;
        /* E   */ 8'h24: ascii = shift ? 8'h45 : 8'h65;
        /* F   */ 8'h2B: ascii = shift ? 8'h46 : 8'h66;
        /* G   */ 8'h34: ascii = shift ? 8'h47 : 8'h67;
        /* H   */ 8'h33: ascii = shift ? 8'h48 : 8'h68;
        /* I   */ 8'h43: ascii = shift ? 8'h49 : 8'h69;
        /* J   */ 8'h3B: ascii = shift ? 8'h4A : 8'h6A;
        /* K   */ 8'h42: ascii = shift ? 8'h4B : 8'h6B;
        /* L   */ 8'h4B: ascii = shift ? 8'h4C : 8'h6C;
        /* M   */ 8'h3A: ascii = shift ? 8'h4D : 8'h6D;
        /* N   */ 8'h31: ascii = shift ? 8'h4E : 8'h6E;
        /* O   */ 8'h44: ascii = shift ? 8'h4F : 8'h6F;
        /* P   */ 8'h4D: ascii = shift ? 8'h50 : 8'h70;
        /* Q   */ 8'h15: ascii = shift ? 8'h51 : 8'h71;
        /* R   */ 8'h2D: ascii = shift ? 8'h52 : 8'h72;
        /* S   */ 8'h1B: ascii = shift ? 8'h53 : 8'h73;
        /* T   */ 8'h2C: ascii = shift ? 8'h54 : 8'h74;
        /* U   */ 8'h3C: ascii = shift ? 8'h55 : 8'h75;
        /* V   */ 8'h2A: ascii = shift ? 8'h56 : 8'h76;
        /* W   */ 8'h1D: ascii = shift ? 8'h57 : 8'h77;
        /* X   */ 8'h22: ascii = shift ? 8'h58 : 8'h78;
        /* Y   */ 8'h35: ascii = shift ? 8'h59 : 8'h79;
        /* Z   */ 8'h1A: ascii = shift ? 8'h5A : 8'h7A;

        // Цифры
        /* 0)  */ 8'h45: ascii = shift ? 8'h29 : 8'h30;
        /* 1!  */ 8'h16: ascii = shift ? 8'h21 : 8'h31;
        /* 2@  */ 8'h1E: ascii = shift ? 8'h40 : 8'h32;
        /* 3#  */ 8'h26: ascii = shift ? 8'h23 : 8'h33;
        /* 4$  */ 8'h25: ascii = shift ? 8'h24 : 8'h34;
        /* 5%  */ 8'h2E: ascii = shift ? 8'h25 : 8'h35;
        /* 6^  */ 8'h36: ascii = shift ? 8'h5E : 8'h36;
        /* 7&  */ 8'h3D: ascii = shift ? 8'h26 : 8'h37;
        /* 8*  */ 8'h3E: ascii = shift ? 8'h2A : 8'h38;
        /* 9(  */ 8'h46: ascii = shift ? 8'h28 : 8'h39;

        // Спецсимволы
        /* `~  */ 8'h0E: ascii = shift ? 8'h7E : 8'h60;
        /* -_  */ 8'h4E: ascii = shift ? 8'h5F : 8'h2D;
        /* =+  */ 8'h55: ascii = shift ? 8'h2B : 8'h3D;
        /* \|  */ 8'h5D: ascii = shift ? 8'h7C : 8'h5C;
        /* [{  */ 8'h54: ascii = shift ? 8'h7B : 8'h5B;
        /* ]}  */ 8'h5B: ascii = shift ? 8'h7D : 8'h5D;
        /* ;:  */ 8'h4C: ascii = shift ? 8'h3A : 8'h3B;
        /* '"  */ 8'h52: ascii = shift ? 8'h22 : 8'h27;
        /* ,<  */ 8'h41: ascii = shift ? 8'h3C : 8'h2C;
        /* .>  */ 8'h49: ascii = shift ? 8'h3E : 8'h2E;
        /* /?  */ 8'h4A: ascii = shift ? 8'h3F : 8'h2F;

        // Специальные клавиши
        /* SH   */ 8'h12, 8'h59: ascii = 8'h01;
        /* ALT  */ 8'h11: ascii = 8'h02;
        /* CTL  */ 8'h14: ascii = 8'h03;
        /* UP   */ 8'h75: ascii = 8'h04;
        /* DN   */ 8'h72: ascii = 8'h05;
        /* LF   */ 8'h6B: ascii = 8'h06;
        /* RT   */ 8'h74: ascii = 8'h07;
        /* BS   */ 8'h66: ascii = 8'h08;
        /* TAB  */ 8'h0D: ascii = 8'h09;
        /* ENT  */ 8'h5A: ascii = 8'h0A;
        /* HOME */ 8'h6C: ascii = 8'h0B;
        /* END  */ 8'h69: ascii = 8'h0C;
        /* PGUP */ 8'h7D: ascii = 8'h0D;
        /* PGDN */ 8'h7A: ascii = 8'h0E;
        /* DEL  */ 8'h71: ascii = 8'h0F;
        /* F1   */ 8'h05: ascii = 8'h10;
        /* F2   */ 8'h06: ascii = 8'h11;
        /* F3   */ 8'h04: ascii = 8'h12;
        /* F4   */ 8'h0C: ascii = 8'h13;
        /* F5   */ 8'h03: ascii = 8'h14;
        /* F6   */ 8'h0B: ascii = 8'h15;
        /* F7   */ 8'h83: ascii = 8'h16;
        /* F8   */ 8'h0A: ascii = 8'h17;
        /* F9   */ 8'h01: ascii = 8'h18;
        /* F10  */ 8'h09: ascii = 8'h19;
        /* F11  */ 8'h78: ascii = 8'h1A;
        /* ESC  */ 8'h76: ascii = 8'h1B;
        /* INS  */ 8'h70: ascii = 8'h1C;
        /* NUM  */ 8'h77: ascii = 8'h1D;
        /* F12  */ 8'h07: ascii = 8'h1E;
        /* SPC  */ 8'h29: ascii = 8'h20;

        /* F0 (Unpressed Signal), E0, E1, ... */
        default: ascii = at;

    endcase

end

endmodule
