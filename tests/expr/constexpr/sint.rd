// test: Test constant expressions with signed integers

pub fn main(): int {
    let a: [3]int;
    let b: [1 as int + 2]int = a;
    let c: [7 as int - 4]int = a;
    let d: [7 as int * 4 / 8]int = a;
    let e: [7 as int % 4]int = a;
    let f: [1 as int | 2]int = a;
    let g: [3 as int & 7]int = a;
    let h: [19 as int ^ 16]int = a;
    return 0;
}
