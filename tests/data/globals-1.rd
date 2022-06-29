// test: Test global variables

let a: int;
let b: int;
let c;

pub fn main(): int {
    a = 5;
    b = 2 * a;
    c = a + b;
    if c != 15 {
        return 1;
    }
    return 0;
}

