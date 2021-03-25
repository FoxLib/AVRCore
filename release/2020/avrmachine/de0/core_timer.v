
// Таймер
reg [14:0] timer_divider = 0;
reg [23:0] timer_ms      = 0;

// Отсчет таймера в миллисекундах
always @(posedge clock) begin

    if (timer_divider == 24999) begin
        timer_divider <= 0;
        timer_ms      <= timer_ms + 1;
    end else
        timer_divider <= timer_divider + 1;
end

// Таймер прерываний
reg        intr_trigger = 0;
reg [ 7:0] intr_maxtime = 0; // Выключен
reg [ 7:0] intr_timer   = 0;
