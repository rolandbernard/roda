// test: Test constant expressions with integers

pub fn main(): int {
    let a: [3]int;
    let b: [1 + 2]int = a;
    let c: [7 - 4]int = a;
    let d: [7 * 4 / 8]int = a;
    let e: [7 % 4]int = a;
    let f: [1 | 2]int = a;
    let g: [3 & 7]int = a;
    let h: [19 ^ 16]int = a;
    return 0;
}
