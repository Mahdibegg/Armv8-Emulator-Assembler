.global _start

_start:
    /* Load GPIO base address 0x3f20 in x0 */
    movz x0, #0                 
    movk x0, #0x3f20, lsl #16   

    /* Load GPFSEL1 register (controls pins 10–19) */
    ldr w1, [x0, #0x04]

    /* Set GPIO16 to output (001 in bits 20–18) */
    mov w2, #(1 << 18)
    orr w1, w1, w2

    /* Store back to GPFSEL1 */
    str w1, [x0, #0x04]

loop:
    /* Turn LED ON */
    mov w1, #(1 << 16)
    str w1, [x0, #0x1c]

    /* Turn LED OFF */
    mov w1, #(1 << 16)
    str w1, [x0, #0x28]