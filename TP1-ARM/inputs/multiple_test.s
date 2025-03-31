.text
    // Test 1: MOVZ (Move with Zero)
    MOVZ    X0, 0x1234                // X0 = 0x1234 (inicialización)
    // Test 2: ADD (Extended Register)
    ADD     X1, X0, X0, LSL #1        // X1 = X0 + (X0 << 1)
    // Test 3: ADD (Immediate)
    ADD     X2, X1, #5                // X2 = X1 + 5
    // Test 4: ADDS (Extended Register)
    ADDS    X3, X2, X1                // X3 = X2 + X1, con actualización de flags
    // Test 5: ADDS (Immediate)
    ADDS    X4, X3, #10               // X4 = X3 + 10, con actualización de flags
    // Test 6: SUBS (Extended Register)
    SUBS    X5, X4, X3                // X5 = X4 - X3, con actualización de flags
    // Test 7: SUBS (Immediate)
    SUBS    X6, X5, #3                // X6 = X5 - 3, con actualización de flags
    // Test 8: CMP (Extended Register)
    CMP     X6, X2                    // CMP X6 con X2 (sin almacenar el resultado)
    // Test 9: CMP (Immediate)
    CMP     X6, #20                   // CMP X6 con 20
    // Test 10: ANDS (Extended Register)
    ANDS    X7, X3, X5                // X7 = X3 & X5, con actualización de flags
    // Test 11: ORR (Extended Register)
    ORR     X8, X7, X1                // X8 = X7 | X1
    // Test 12: EOR (Extended Register)
    EOR     X9, X8, X3                // X9 = X8 ^ X3
    // Test 13: MUL (Multiplicación)
    MUL     X10, X1, X2               // X10 = X1 * X2
    // Test 14: LSL (Immediate)
    LSL     X11, X10, #3              // X11 = X10 << 3
    // Test 15: LSR (Immediate)
    LSR     X12, X11, #2              // X12 = X11 >> 2
    // Test 16: STUR (Store a word)
    STUR    X11, [X0, #8]             // M[X0 + 0x8] = X11
    // Test 17: STURH (Store a halfword)
    STURH   W12, [X0, #16]            // M[X0 + 0x10] = W12
    // Test 18: STURB (Store a byte)
    STURB   W13, [X0, #24]            // M[X0 + 0x18] = W13
    // Test 19: LDUR (Load a word)
    LDUR    X14, [X0, #8]             // X14 = M[X0 + 0x8]
    // Test 20: LDURH (Load a halfword)
    LDURH   W15, [X0, #16]            // W15 = M[X0 + 0x10]
    // Test 21: LDURB (Load a byte)
    LDURB   W16, [X0, #24]            // W16 = M[X0 + 0x18]
    // Test 22: CBZ (Compare and Branch on Zero)
    CBNZ     X14, branch_here          // Salta a branch_here si X14 es 0
    // Test 23: CBNZ (Compare and Branch on Non-Zero)
    CBZ    X15, branch_here          // Salta a branch_here si X15 no es 0
    // Test 24: B (Unconditional Branch)
    B       end                       // Salta a end
branch_here:
    // Test 25: BGT (Branch if Greater Than)
    BGT     greater_than              // Salta si X1 > X2
    // Test 26: BLT (Branch if Less Than)
    BLT     less_than                 // Salta si X1 < X2
    // Test 27: BGE (Branch if Greater or Equal)
    BGE     greater_equal             // Salta si X1 >= X2
    // Test 28: BNE (Branch if Not Equal)
    BNE     not_equal                 // Salta si X1 != X2
    // Test 29: BEQ (Branch if Equal)
    BEQ     equal                     // Salta si X1 == X2

greater_than:
    // Test 30: BR (Branch Register)
    BR      X16                       // Salta a la dirección contenida en X16
    B       end                       // Salta a end

less_than:
    BR      X15
    B       end

greater_equal:
    BR      X14
    B       end

not_equal:
    BR      X13
    B       end

equal:
    BR      X12
    B       end

end:
    // Test 31: HLT (Halt)
    HLT     0                         // Detener la simulación
