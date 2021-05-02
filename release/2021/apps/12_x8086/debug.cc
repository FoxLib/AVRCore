
// Отладка: выдать регистры и "повиснуть"
void dumpregs() {

    D.locate(0, 1);

    D.print(" AX="); D.print(regs[0], 16, 2);
    D.print(" BX="); D.print(regs[3], 16, 2);
    D.print(" CX="); D.print(regs[1], 16, 2);
    D.print(" DX="); D.print(regs[2], 16, 2); D.print("\n");
    D.print(" SP="); D.print(regs[4], 16, 2);
    D.print(" BP="); D.print(regs[5], 16, 2);
    D.print(" SI="); D.print(regs[6], 16, 2);
    D.print(" DI="); D.print(regs[7], 16, 2); D.print("\n");
    D.print(" SS="); D.print(segs[2], 16, 2);
    D.print(" CS="); D.print(segs[1], 16, 2);
    D.print(" DS="); D.print(segs[3], 16, 2);
    D.print(" ES="); D.print(segs[0], 16, 2); D.print("\n");
    D.print(" FL="); D.print(flags, 16, 2);
    D.print(" IP="); D.print(ip, 16, 2); D.print(" ");

    D.print(flags & V_FLAG ? "O" : ".");
    D.print(flags & D_FLAG ? "D" : ".");
    D.print(flags & I_FLAG ? "I" : ".");
    D.print(flags & T_FLAG ? "T" : ".");
    D.print(flags & N_FLAG ? "S" : ".");
    D.print(flags & Z_FLAG ? "Z" : ".");
    D.print(flags & A_FLAG ? "A" : ".");
    D.print(flags & P_FLAG ? "P" : ".");
    D.print(flags & C_FLAG ? "C" : "."); D.print(" ");

    D.print("tstates: [ "); D.print(tstates); D.print(" ]");

    for(;;);
}
