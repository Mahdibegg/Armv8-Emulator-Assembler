_start:
    movz x0, #0                 
    movk x0, #0x3f20, lsl #16   

    ldr w1, [x0, #0x04]

    movz w2, #0x100, lsl #16
    orr w1, w1, w2

    str w1, [x0, #0x04]

    movz w1, #0x4, lsl #16

loop:
    str w1, [x0, #0x1c]
    movz x3, #0x120, lsl #16

on_delay:
    subs x3, x3, #1
    b.ne on_delay

    str w1, [x0, #0x28]
    movz x3, #0x120, lsl #16

off_delay:
    subs x3, x3, #1
    b.ne off_delay

    b loop