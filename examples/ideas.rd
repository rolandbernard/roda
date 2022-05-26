
fn fib(n: u64): u64 {
    let a, b = 0, 1;
    for _ in 0..n {
        a, b = b, a + b;
    }
    return a;
}

fn fac(n: u64): u64 {
    if n == 0 || n == 1 {
        return 1;
    } else {
        return n * fac(n - 1);
    }
}

record House {
    a: u32,
    b: u16
}

type Test = {a: u32, b: i32};

import fn print(s: *const u8);

export fn main() {
    print("Hello World!");
    let i: u32 = 0;
    while i < 5 {
        i += 1;
    }
    loop {

        if i == 10 {
            break;
        }
    }
    do {
        
    } while i == 10;
    match i {
       0 => {

       }
       _ => {
           
       }
    }
    let arr: [5]int;
    let arr: [5]uint;
}

i8 i16 i32 i64 int
u8 u16 u32 u64 uint
f32 f64
*u8
[N]T
type Test = (a: i32, b: f32);

// Variable names:
_testTest1234


functionName
TypeName
var_name
MACRO_NAME ENUM_NAME


