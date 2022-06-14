
/*
    This is a block comment.
    /* 
        They can also be nested.
        /* 
            As deep as you like. /* HELLO WORLD! */
         */
     */
 */

extern fn exit(status: int): !;

extern fn print(text: *u8);

pub fn main(argc: int, argv: **u8): int {
    let a: int = 5;
    let b = 7;
    let c = a + b;
    let d: [5 + 7 / 2]int;
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

fn add(a: int, b: int): int {
    return a + b;
}

