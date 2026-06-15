.global _start

_start:
    movz x0, #0                 /* Set register 0 to 0 */
    movk x0, #0x3f20, lsl #16   /* Store GPIO base address 0x3f20 in x0 */

loop:
    b loop