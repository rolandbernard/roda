
/*
    This is a block comment.
    /* 
        They can also be nested.
        /* 
            As deep as you like. /* HELLO WORLD! */
         */
     */
 */

import fn exit(status: int): !;

import fn print(text: *u8);

export fn main(argc: int, argv: **u8): int {
    let a: int = 5;
    let b = 7;
    let c = a + b;
    let d: [5]int;
    d[0] = a;
    d[1] = b;
    d[2] = c;
    d[3] = add(a, b);
    d[4] = add(add(a, b), c);
    if !(a + b == c) {
        return 1;
    } else {
        return 0;
    }
}

fn add(a: u32, b: u32): u32 {
    return a + b;
}

