
extern fn print(text: *u8);

/*
   This is a block comment.
   /* They can also be nested. /* As deep as you like. */ */
 */

fn fma(a: uint, b: uint /* Hello world! */, c: uint): uint {
    while a < b {
        b += 1;
        a *= 2;
    }
    return a * b + c;
}

type Test1 = int;
type Test2 = Test1;
type Test3 = Test2;

pub fn main(argc: int, argv: **u8): Test1 {
    /* Print some stuff! */
    print("Hello \t \"world\"!");
    print("猕猴桃");     // Supports utf8 in string literals.
    print("\U0001F600"); // Or if you like, you can use an escape sequence.
    print(5.2E-2);
    print(829343.545);
    print(829343);
    print(0b1001001 + 0o01247 + 0d0429 + 0hFF09ab + 0xFF09ab);      // Integer literals in different bases
    let test = 1_000_000;
    test += 1_000.123_56e-1_2;
    return 0;
}

