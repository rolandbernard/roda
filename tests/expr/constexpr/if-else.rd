// test: Test constant expressions with if-else expressions

pub fn main(): int {
    let a: [3]int;
    let b: [if true { 3 } else { 5 }]int = a;
    let c: [if false { 5 } else { 3 }]int = a;
    let d: [if 5 == 7 { 5 * 3 } else { 7 - 4 }]int = a;
    let e: [if 5 == 5 { (5 + 4) / 3 } else { 7 }]int = a;
    let f: [if 5 * 3 == 30 / 2 { (5 + 10) / 5 } else { 7 }]int = a;
    let f: [if false || true { (5 + 10) / 5 } else { 7 }]int = a;
    let f: [if true || false { (5 + 10) / 5 } else { 7 }]int = a;
    let f: [if true && false { 5 } else { 3 }]int = a;
    return 0;
}
