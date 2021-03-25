// ---------------------------------------------------------------------
// Реализация прерывания
// ---------------------------------------------------------------------

else if (intr_trigger)
case (tstate)

    // Запись PCL
    0: begin

        tstate  <= 1;
        address <= sp;
        wb      <= pc[7:0];
        w       <= 1'b1;
        sp_mth  <= `SPDEC;

    end

    // Запись PCH
    1: begin

        tstate  <= 0;
        address <= sp;
        wb      <= pcnext[15:8];
        w       <= 1'b1;
        sp_mth  <= `SPDEC;
        pc      <= `INT0_VECT;           // AVR8K ISR(INT0_vect)

        // Сброс флага I->0 (sreg)
        alu     <= 11;
        op2     <= {1'b0, sreg[6:0]};
        sreg_w  <= 1'b1;

        // Переход к обычному исполнению
        intr_trigger <= 0;

    end

endcase

// Есть истечение времени работы инструкции и вызов таймера
else if (tstate == 0 && sreg[7] && (timer_ms[7:0] - intr_timer > intr_maxtime)) begin

    intr_trigger <= 1'b1;
    intr_timer   <= timer_ms[7:0];

end
