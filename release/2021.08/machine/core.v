module core
(
    // Основной интерфейс
    input       wire        clock,          // Тактовый генератор
    output      reg  [15:0] pc,             // Адрес PC
    output      reg  [15:0] address,        // Запрос в память
    input       wire [15:0] ir,             // Instruction Register
    input       wire [ 7:0] i_data,         // Считывание данных
    output      reg  [ 7:0] o_data,         // Запись данных
    output      reg         we              // Разрешение записи
);

initial begin we = 0; o_data = 0; pc = 0; address = 0; end

endmodule
