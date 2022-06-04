
/*
    This is a block comment.
    /* 
        They can also be nested.
        /* 
            As deep as you like. /* HELLO WORLD! */
         */
     */
 */

fn add(a: u32, b: u32): u32 {
    return a + b;
}

import fn exit(status: int): !;

import fn print(text: *u8);

export fn main(argc: int, argv: **u8): int {
    let a: int = 5;
    let b = 7;
    let c = a + b;
    if !(a + b == c) {
        return 1;
    } else {
        return 0;
    }
}

