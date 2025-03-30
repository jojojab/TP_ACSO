.text
    // Configurar base y datos
    MOVZ X0, #0x1000            // X0 = dirección base 0x1000
    MOVZ W1, #0xABCD            // W1 = 0xABCD (32 bits para STURB/STURH)
    MOVZ X2, #0xDEF0            // Parte baja de X2
    MOVZ X2, #0x9ABC, LSL #16   // Parte media-baja
    MOVZ X2, #0x5678, LSL #32   // Parte media-alta
    MOVZ X2, #0x1234, LSL #48   // X2 = 0x123456789ABCDEF0

    // STUR (64-bit) alineado
    STUR X2, [X0, #0]           // Guardar 0x123456789ABCDEF0 en 0x1000

    // LDUR (64-bit) alineado
    LDUR X3, [X0, #0]           // Cargar desde 0x1000 a X3

    // STURB (8-bit) no alineado
    STURB W1, [X0, #1]          // Guardar 0xCD en 0x1001

    // LDURB (8-bit) no alineado
    LDURB W4, [X0, #1]          // Cargar byte desde 0x1001 a W4

    // STURH (16-bit) cruzando límite
    STURH W1, [X0, #3]          // Guardar 0xABCD en 0x1003-0x1004 (cruza 32-bit)

    // LDURH (16-bit) cruzando límite
    LDURH W5, [X0, #3]          // Cargar halfword desde 0x1003 a W5

    // STUR no alineado
    STUR X2, [X0, #1]           // Guardar 0x123456789ABCDEF0 en 0x1001 (no alineado)

    // LDUR no alineado
    LDUR X6, [X0, #1]           // Cargar desde 0x1001

    BRK #0                      // Fin del programa