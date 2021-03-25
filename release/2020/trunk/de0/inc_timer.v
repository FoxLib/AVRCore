
// Отсчет таймера в миллисекундах
always @(posedge clock) begin

    if (timer_divider == 24999) begin
        timer_divider <= 0;
        timer_ms      <= timer_ms + 1;
    end else
        timer_divider <= timer_divider + 1;
end
