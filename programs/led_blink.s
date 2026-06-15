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

    /* pin-16 mask for both GPSET0 and GPCLR0 (bit 16) - never changes */
    movz w1, #0x1, lsl #16      /* 1 << 16 */

loop:
    /* Turn LED ON */
    str w1, [x0, #0x1c]         /* GPSET0 */
    movz x3, #0x800, lsl #16

    /* Turn LED OFF */
    str w1, [x0, #0x28]         /* GPCLR0 */
    movz x3, #0x800, lsl #16