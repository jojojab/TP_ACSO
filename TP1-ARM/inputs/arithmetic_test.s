.text
    // Inicializar registros
    MOVZ X0, #0x1234            // X0 = 0x1234
    MOVZ X1, #0x5678            // X1 = 0x5678

    // ADDS con registros
    ADDS X2, X0, X1             // X2 = X0 + X1 = 0x68AC, probar flags Z y N

    // SUBS con registros
    SUBS X3, X1, X0             // X3 = X1 - X0 = 0x4444, probar flags

    // MUL
    MUL X4, X0, X1              // X4 = X0 * X1 = 0x626FB60

    // ADDS con inmediato
    ADDS X5, X0, #0x100         // X5 = X0 + 0x100 = 0x1334

    // SUBS con inmediato y shift
    SUBS X6, X1, #0x1, LSL #12  // X6 = X1 - 0x1000 = 0x4678

    // ADD con inmediato
    ADD X7, X0, #0x1000         // X7 = X0 + 0x1000 = 0x2234

    // ADD con extensión (UXTW)
    ADD X8, X0, X1, UXTW #2     // X8 = X0 + (X1 & 0xFFFFFFFF) << 2 = 0x17B34

    // ANDS con shift
    ANDS X9, X0, X1, LSL #4     // X9 = X0 & (X1 << 4), probar flags

    // EOR con shift
    EOR X10, X0, X1, LSR #2     // X10 = X0 ^ (X1 >> 2)

    // ORR con shift
    ORR X11, X0, X1, ASR #1     // X11 = X0 | (X1 >> 1 con signo)

    HLT 0                        // Fin del programa